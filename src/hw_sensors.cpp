#include "hw_sensors.h"
#include "robot_config.h"
#include "logic_imu.h"
#include "logic_ir.h"
#include <Wire.h>

// ==========================================================
// ثوابت خاصة بالـ IMU (MPU6500 / GY-6500)
// ==========================================================
#define MPU_REG_PWR_MGMT_1   0x6B
#define MPU_REG_ACCEL_XOUT_H 0x3B
#define GYRO_SENS_LSB_PER_DPS 131.0f   // مدى ±250 deg/s (الافتراضي)
#define ACCEL_SENS_LSB_PER_G  16384.0f // مدى ±2g (الافتراضي)

static float g_yaw = 0.0f;
static float g_gyro_z_offset = 0.0f;

// قراءة عدد بايتات من رجستر معين
static void mpu_read_bytes(uint8_t reg, uint8_t* buf, uint8_t len) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)IMU_I2C_ADDR, (uint8_t)len, true);
    for (uint8_t i = 0; i < len && Wire.available(); i++) {
        buf[i] = Wire.read();
    }
}

static void mpu_write_byte(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission(true);
}

// ==========================================================
// قراءة حساس IR واحد مع إلغاء الضوء المحيط
// بيقيس القراءة والـ emitter شغال، بعدين والـ emitter مطفي،
// والفرق بينهم هو قيمة الانعكاس الحقيقية من الحيطة
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
    // بينات الـ IR
    pinMode(IR_LEFT_TX_PIN, OUTPUT);
    pinMode(IR_FRONT_TX_PIN, OUTPUT);
    pinMode(IR_RIGHT_TX_PIN, OUTPUT);
    digitalWrite(IR_LEFT_TX_PIN, LOW);
    digitalWrite(IR_FRONT_TX_PIN, LOW);
    digitalWrite(IR_RIGHT_TX_PIN, LOW);

    // I2C للـ IMU
    Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
    Wire.setClock(400000);
    mpu_write_byte(MPU_REG_PWR_MGMT_1, 0x00); // إيقاظ الـ IMU من sleep mode
    delay(100);

    sensors_calibrate_gyro();
}

// ==========================================================
// قراءة الـ IR الثلاثة
// ==========================================================
IRReadings sensors_read_ir() {
    IRReadings r;
    r.left  = read_ir_sensor(IR_LEFT_TX_PIN,  IR_LEFT_RX_PIN);
    r.front = read_ir_sensor(IR_FRONT_TX_PIN, IR_FRONT_RX_PIN);
    r.right = read_ir_sensor(IR_RIGHT_TX_PIN, IR_RIGHT_RX_PIN);
    return r;
}

// ==========================================================
// قراءة الـ IMU الخام (تسارع + جايروسكوب)
// ==========================================================
IMUData sensors_read_imu() {
    uint8_t buf[14];
    mpu_read_bytes(MPU_REG_ACCEL_XOUT_H, buf, 14);

    int raw_ax = imu_combine_bytes(buf[0], buf[1]);
    int raw_ay = imu_combine_bytes(buf[2], buf[3]);
    int raw_az = imu_combine_bytes(buf[4], buf[5]);
    // buf[6],buf[7] = temperature، مش محتاجينها دلوقتي
    int raw_gx = imu_combine_bytes(buf[8], buf[9]);
    int raw_gy = imu_combine_bytes(buf[10], buf[11]);
    int raw_gz = imu_combine_bytes(buf[12], buf[13]);

    IMUData d;
    d.ax = imu_raw_to_g(raw_ax, ACCEL_SENS_LSB_PER_G);
    d.ay = imu_raw_to_g(raw_ay, ACCEL_SENS_LSB_PER_G);
    d.az = imu_raw_to_g(raw_az, ACCEL_SENS_LSB_PER_G);
    d.gx = imu_raw_to_dps(raw_gx, GYRO_SENS_LSB_PER_DPS);
    d.gy = imu_raw_to_dps(raw_gy, GYRO_SENS_LSB_PER_DPS);
    d.gz = imu_raw_to_dps(raw_gz, GYRO_SENS_LSB_PER_DPS) - g_gyro_z_offset;
    d.yaw = g_yaw;
    return d;
}

// ==========================================================
// تكامل الجايروسكوب لحساب الـ yaw - استدعيها كل loop
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
// معايرة الجايروسكوب - لازم الروبوت يكون واقف ساكن تمامًا
// ==========================================================
void sensors_calibrate_gyro() {
    const int samples = 200;
    float sum = 0.0f;
    uint8_t buf[14];

    for (int i = 0; i < samples; i++) {
        mpu_read_bytes(MPU_REG_ACCEL_XOUT_H, buf, 14);
        int raw_gz = imu_combine_bytes(buf[12], buf[13]);
        sum += imu_raw_to_dps(raw_gz, GYRO_SENS_LSB_PER_DPS);
        delay(3);
    }
    g_gyro_z_offset = sum / samples;
}