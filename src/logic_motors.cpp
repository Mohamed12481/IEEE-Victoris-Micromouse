#include "logic_motors.h"

MotorDriveSignal compute_motor_drive_signal(int pwm, int max_magnitude) {
    MotorDriveSignal signal;

    if (pwm > max_magnitude)  pwm = max_magnitude;
    if (pwm < -max_magnitude) pwm = -max_magnitude;

    if (pwm >= 0) {
        signal.in1_high = true;
        signal.in2_high = false;
        signal.pwm_magnitude = pwm;
    } else {
        signal.in1_high = false;
        signal.in2_high = true;
        signal.pwm_magnitude = -pwm;
    }

    return signal;
}