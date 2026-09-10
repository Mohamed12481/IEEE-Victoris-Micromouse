#include "logic_imu.h"

int imu_combine_bytes(unsigned char high_byte, unsigned char low_byte) {
    short combined = (short)((high_byte << 8) | low_byte);
    return (int)combined;
}

float imu_raw_to_g(int raw, float lsb_per_g) {
    return (float)raw / lsb_per_g;
}

float imu_raw_to_dps(int raw, float lsb_per_dps) {
    return (float)raw / lsb_per_dps;
}

float integrate_yaw(float current_yaw, float gyro_z_dps, float dt) {
    return current_yaw + (gyro_z_dps * dt);
}