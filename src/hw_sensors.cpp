#include "hw_sensors.h"
#include "robot_config.h"
#include "logic_imu.h"
#include "logic_wall_detection.h"
#include "logic_ir.h"
#include <Wire.h>

#define BMI160_REG_CHIP_ID       0x00
#define BMI160_CHIP_ID           0xD1
#define BMI160_REG_GYR_X_LSB     0x0C
#define BMI160_REG_ACC_CONF      0x40
#define BMI160_REG_ACC_RANGE     0x41
#define BMI160_REG_GYR_CONF      0x42
#define BMI160_REG_GYR_RANGE     0x43
#define BMI160_REG_CMD           0x7E

#define BMI160_CMD_ACC_NORMAL    0x11
#define BMI160_CMD_GYR_NORMAL    0x15
#define BMI160_CMD_SOFT_RESET    0xB6

#define BMI160_ACC_CONF_100HZ    0x28
#define BMI160_ACC_RANGE_2G      0x03
#define BMI160_GYR_CONF_100HZ    0x28
#define BMI160_GYR_RANGE_250DPS  0x03
#define BMI160_GYRO_LSB_PER_DPS  131.2f
#define BMI160_ACCEL_LSB_PER_G   16384.0f

// ==========================================================
// BMI160 IMU configuration and scaling.
// ==========================================================

static float g_yaw = 0.0f;
static float g_gyro_z_offset = 0.0f;
static bool g_imu_ready = false;

// قراءة عدد بايتات من رجستر معين
static bool bmi160_read_bytes(uint8_t reg, uint8_t* buf, size_t len) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    size_t received = Wire.requestFrom((uint8_t)IMU_I2C_ADDR, len, true);
    if (received != len) {
        while (Wire.available()) {
            Wire.read();
        }
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        if (!Wire.available()) {
            return false;
        }
        buf[i] = Wire.read();
    }
    return true;
}

static bool bmi160_write_byte(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission(true) == 0;
}

static bool bmi160_write_and_verify(uint8_t reg, uint8_t value) {
    uint8_t readback = 0;
    return bmi160_write_byte(reg, value) &&
           bmi160_read_bytes(reg, &readback, 1) &&
           readback == value;
}

static bool bmi160_initialize() {
    uint8_t chip_id = 0;

    Wire.beginTransmission(IMU_I2C_ADDR);
    if (Wire.endTransmission(true) != 0 ||
        !bmi160_read_bytes(BMI160_REG_CHIP_ID, &chip_id, 1) ||
        chip_id != BMI160_CHIP_ID) {
        Serial.println("[BMI160] Device or CHIP_ID not found.");
        return false;
    }

    if (!bmi160_write_byte(BMI160_REG_CMD, BMI160_CMD_SOFT_RESET)) {
        Serial.println("[BMI160] Soft reset failed.");
        return false;
    }
    delay(100);

    if (!bmi160_write_byte(BMI160_REG_CMD, BMI160_CMD_ACC_NORMAL)) {
        Serial.println("[BMI160] Accelerometer normal-mode command failed.");
        return false;
    }
    delay(5);

    if (!bmi160_write_byte(BMI160_REG_CMD, BMI160_CMD_GYR_NORMAL)) {
        Serial.println("[BMI160] Gyroscope normal-mode command failed.");
        return false;
    }
    delay(100);

    if (!bmi160_write_and_verify(BMI160_REG_ACC_CONF, BMI160_ACC_CONF_100HZ) ||
        !bmi160_write_and_verify(BMI160_REG_ACC_RANGE, BMI160_ACC_RANGE_2G) ||
        !bmi160_write_and_verify(BMI160_REG_GYR_CONF, BMI160_GYR_CONF_100HZ) ||
        !bmi160_write_and_verify(BMI160_REG_GYR_RANGE, BMI160_GYR_RANGE_250DPS)) {
        Serial.println("[BMI160] Configuration verification failed.");
        return false;
    }

    Serial.println("[BMI160] Initialized.");
    return true;
}

// ==========================================================
// قراءة حساس IR واحد مع إلغاء الضوء المحيط
// ==========================================================
static int read_ir_sensor(uint8_t tx_pin, uint8_t rx_pin) {
    digitalWrite(tx_pin, HIGH);
    delayMicroseconds(200);          // وقت استقرار الـ emitter
    int on_reading = analogRead(rx_pin);
    digitalWrite(tx_pin, LOW);
    delayMicroseconds(200);
    int ambient_reading = analogRead(rx_pin);

    return cancel_ambient_light(on_reading, ambient_reading);
}

// ==========================================================
// التهيئة
// ==========================================================
void sensors_init() {
    // بين باعثات الـ IR (MOSFET Gate)
    pinMode(IR_EMITTERS_PIN, OUTPUT);
    digitalWrite(IR_EMITTERS_PIN, LOW);

    // I2C للـ IMU
    Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
    Wire.setClock(400000);
    g_imu_ready = bmi160_initialize();
    delay(100);

    if (g_imu_ready) {
        sensors_calibrate_gyro();
    }
}

// ==========================================================
// قراءة الـ IR الثلاثة
// ==========================================================
IRReadings sensors_read_ir() {
    IRReadings r;
    r.left  = read_ir_sensor(IR_EMITTERS_PIN, IR_LEFT_RX_PIN);
    r.front = read_ir_sensor(IR_EMITTERS_PIN, IR_FRONT_RX_PIN);
    r.right = read_ir_sensor(IR_EMITTERS_PIN, IR_RIGHT_RX_PIN);
    return r;
}

// ==========================================================
// قراءة الـ IMU الخام (تسارع + جايروسكوب)
// ==========================================================
IMUData sensors_read_imu() {
    IMUData d = {};
    d.yaw = g_yaw;

    uint8_t buf[12];
    if (!g_imu_ready || !bmi160_read_bytes(BMI160_REG_GYR_X_LSB, buf, sizeof(buf))) {
        return d;
    }

    int raw_ax = imu_combine_bytes(buf[7], buf[6]);
    int raw_ay = imu_combine_bytes(buf[9], buf[8]);
    int raw_az = imu_combine_bytes(buf[11], buf[10]);
    int raw_gx = imu_combine_bytes(buf[1], buf[0]);
    int raw_gy = imu_combine_bytes(buf[3], buf[2]);
    int raw_gz = imu_combine_bytes(buf[5], buf[4]);

    d.ax = imu_raw_to_g(raw_ax, BMI160_ACCEL_LSB_PER_G);
    d.ay = imu_raw_to_g(raw_ay, BMI160_ACCEL_LSB_PER_G);
    d.az = imu_raw_to_g(raw_az, BMI160_ACCEL_LSB_PER_G);
    d.gx = imu_raw_to_dps(raw_gx, BMI160_GYRO_LSB_PER_DPS);
    d.gy = imu_raw_to_dps(raw_gy, BMI160_GYRO_LSB_PER_DPS);
    d.gz = imu_raw_to_dps(raw_gz, BMI160_GYRO_LSB_PER_DPS) - g_gyro_z_offset;
    return d;
}

// ==========================================================
// تكامل الجايروسكوب لحساب الـ yaw
// ==========================================================
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
// معايرة الجايروسكوب
// ==========================================================
void sensors_calibrate_gyro() {
    if (!g_imu_ready) {
        g_gyro_z_offset = 0.0f;
        return;
    }

    const int samples = 200;
    float sum = 0.0f;
    int valid_samples = 0;
    uint8_t buf[12];

    for (int i = 0; i < samples; i++) {
        if (bmi160_read_bytes(BMI160_REG_GYR_X_LSB, buf, sizeof(buf))) {
            int raw_gz = imu_combine_bytes(buf[5], buf[4]);
            sum += imu_raw_to_dps(raw_gz, BMI160_GYRO_LSB_PER_DPS);
            valid_samples++;
        }
        delay(3);
    }
    g_gyro_z_offset = valid_samples > 0 ? sum / valid_samples : 0.0f;
}