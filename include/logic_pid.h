#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

// ==========================================================
// PID مستقل تمامًا - مفيهوش أي اعتماد على Arduino أو أي هاردوير
// عشان كده ينفع يتاختبر native (على الكمبيوتر) من غير ESP32 خالص
// ==========================================================

struct PIDController {
    float kp, ki, kd;
    float integral;
    float prev_error;
    float integral_limit;
};

PIDController pid_create(float kp, float ki, float kd, float integral_limit = 1000.0f);
float pid_compute(PIDController* pid, float setpoint, float measurement, float dt);
void  pid_reset(PIDController* pid);

#endif // PID_CONTROLLER_H