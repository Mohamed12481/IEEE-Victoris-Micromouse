// ===== FILE: src/hw_sensors.cpp (MPU6050 Updated) =====

#include "hw_sensors.h"
#include "robot_config.h"
#include "logic_imu.h"
#include "logic_wall_detection.h"
#include "logic_ir.h"
#include <Wire.h>

// ==========================================================
// MPU6050 Register Definitions & Scaling
// ==========================================================
#define MPU6050_ADDR            0x68
#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_ACCEL_CONFIG    0x1C
#define MPU6050_GYRO_CONFIG     0x1B
#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_GYRO_XOUT_H     0x43

#define MPU6050_GYRO_LSB_PER_DPS  131.0f  // Range +/-250 dps
#define MPU6050_ACCEL_LSB_PER_G   16384.0f // Range +/-2g

static float g_yaw = 0.0f;
static float g_gyro_z_offset = 0.0f;
static bool g_imu_ready = false;

// دالة كتابة بايت لرجستر في MPU6050
static bool mpu6050_write_byte(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission(true) == 0);
}

// دالة قراءة عدة بايتات متتالية من MPU6050
static bool mpu6050_read_bytes(uint8_t reg, uint8_t* buf, size_t len) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;

    size_t received = Wire.requestFrom((uint8_t)IMU_I2C_ADDR, len, true);
    if (received != len) return false;

    for (size_t i = 0; i < len; i++) {
        buf[i] = Wire.read();
    }
    return true;
}

// تهيئة حساس MPU6050 وإيقاف وضع النوم (Sleep Mode)
static bool mpu6050_initialize() {
    Wire.beginTransmission(IMU_I2C_ADDR);
    if (Wire.endTransmission(true) != 0) {
        Serial.println("[MPU6050] Device not found on I2C bus!");
        return false;
    }

    // إيقاظ MPU6050 من وضع النوم (Power Management 1)
    if (!mpu6050_write_byte(MPU6050_PWR_MGMT_1, 0x00)) {
        Serial.println("[MPU6050] Wake up failed.");
        return false;
    }
    delay(50);

    // ضبط المدى (Gyro: +/-250 dps, Accel: +/-2g)
    mpu6050_write_byte(MPU6050_GYRO_CONFIG, 0x00);
    mpu6050_write_byte(MPU6050_ACCEL_CONFIG, 0x00);

    Serial.println("[MPU6050] Initialized successfully.");
    return true;
}

// ==========================================================
// قراءة حساس الـ IR
// ==========================================================
static int read_ir_sensor(uint8_t tx_pin, uint8_t rx_pin) {
    digitalWrite(tx_pin, HIGH);
    delayMicroseconds(500);
    int on_reading = analogRead(rx_pin);

    digitalWrite(tx_pin, LOW);
    delayMicroseconds(500);
    int ambient_reading = analogRead(rx_pin);

    return cancel_ambient_light(on_reading, ambient_reading);
}

void sensors_init() {
    pinMode(IR_EMITTERS_PIN, OUTPUT);
    digitalWrite(IR_EMITTERS_PIN, LOW);

    pinMode(IR_LEFT_RX_PIN, INPUT);
    pinMode(IR_FRONT_RX_PIN, INPUT);
    pinMode(IR_RIGHT_RX_PIN, INPUT);

    // تهيئة الـ I2C للـ MPU6050
    Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
    Wire.setClock(400000);
    
    g_imu_ready = mpu6050_initialize();
    delay(100);

    if (g_imu_ready) {
        sensors_calibrate_gyro();
    }
}

IRReadings sensors_read_ir() {
    IRReadings r;
    r.left  = read_ir_sensor(IR_EMITTERS_PIN, IR_LEFT_RX_PIN);
    r.front = read_ir_sensor(IR_EMITTERS_PIN, IR_FRONT_RX_PIN);
    r.right = read_ir_sensor(IR_EMITTERS_PIN, IR_RIGHT_RX_PIN);
    return r;
}

// ==========================================================
// قراءة بيانات MPU6050 الخام (14 بايت: التسارع + الحرارة + الجايروسكوب)
// ==========================================================
IMUData sensors_read_imu() {
    IMUData d = {};
    d.yaw = g_yaw;

    uint8_t buf[14];
    if (!g_imu_ready || !mpu6050_read_bytes(MPU6050_ACCEL_XOUT_H, buf, sizeof(buf))) {
        return d;
    }

    // تجميع البايتات (Big-Endian في MPU6050: MSB أولاً ثم LSB)
    int16_t raw_ax = (buf[0] << 8) | buf[1];
    int16_t raw_ay = (buf[2] << 8) | buf[3];
    int16_t raw_az = (buf[4] << 8) | buf[5];
    
    int16_t raw_gx = (buf[8] << 8) | buf[9];
    int16_t raw_gy = (buf[10] << 8) | buf[11];
    int16_t raw_gz = (buf[12] << 8) | buf[13];

    d.ax = (float)raw_ax / MPU6050_ACCEL_LSB_PER_G;
    d.ay = (float)raw_ay / MPU6050_ACCEL_LSB_PER_G;
    d.az = (float)raw_az / MPU6050_ACCEL_LSB_PER_G;

    d.gx = (float)raw_gx / MPU6050_GYRO_LSB_PER_DPS;
    d.gy = (float)raw_gy / MPU6050_GYRO_LSB_PER_DPS;
    d.gz = ((float)raw_gz / MPU6050_GYRO_LSB_PER_DPS) - g_gyro_z_offset;

    return d;
}

void sensors_update_yaw(float dt) {
    IMUData d = sensors_read_imu();
    g_yaw = integrate_yaw(g_yaw, d.gz, dt);
}

float sensors_get_yaw() {
    return g_yaw;
}

void sensors_reset_yaw() {
    g_yaw = 0.0f;
}

// ==========================================================
// معايرة الجايروسكوب في حالة السكون
// ==========================================================
void sensors_calibrate_gyro() {
    if (!g_imu_ready) {
        g_gyro_z_offset = 0.0f;
        return;
    }

    const int samples = 200;
    float sum = 0.0f;
    int valid_samples = 0;
    uint8_t buf[6];

    for (int i = 0; i < samples; i++) {
        // قراءة الـ Gyro Z مباشرة من رجستر 0x47
        if (mpu6050_read_bytes(0x47, buf, 2)) {
            int16_t raw_gz = (buf[0] << 8) | buf[1];
            sum += ((float)raw_gz / MPU6050_GYRO_LSB_PER_DPS);
            valid_samples++;
        }
        delay(3);
    }
    g_gyro_z_offset = valid_samples > 0 ? sum / valid_samples : 0.0f;
}