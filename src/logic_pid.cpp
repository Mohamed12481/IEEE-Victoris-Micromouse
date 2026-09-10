#include "logic_pid.h"

PIDController pid_create(float kp, float ki, float kd, float integral_limit) {
    PIDController pid;
    pid.kp = kp;
    pid.ki = ki;
    pid.kd = kd;
    pid.integral = 0.0f;
    pid.prev_error = 0.0f;
    pid.integral_limit = integral_limit;
    return pid;
}

float pid_compute(PIDController* pid, float setpoint, float measurement, float dt) {
    if (dt <= 0.0f) return 0.0f; // حماية من القسمة على صفر

    float error = setpoint - measurement;
    
    // حساب الجزء التكاملي مع حماية (Integral Windup)
    pid->integral += error * dt;
    if (pid->integral > pid->integral_limit) pid->integral = pid->integral_limit;
    else if (pid->integral < -pid->integral_limit) pid->integral = -pid->integral_limit;

    // حساب الجزء التفاضلي
    float derivative = (error - pid->prev_error) / dt;
    
    // حفظ الخطأ للخطوة القادمة
    pid->prev_error = error;

    // معادلة الـ PID الكلاسيكية
    return (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);
}

void pid_reset(PIDController* pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}