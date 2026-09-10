#ifndef MOTOR_MATH_H
#define MOTOR_MATH_H

// ==========================================================
// منطق تحويل قيمة PWM موقّعة لإشارة تحكم في H-Bridge - مستقل عن أي هاردوير
// ==========================================================

struct MotorDriveSignal {
    bool in1_high;      // حالة IN1 (true = HIGH)
    bool in2_high;       // حالة IN2 (true = HIGH)
    int  pwm_magnitude;  // قيمة الـ PWM الموجبة (بعد حصرها في المدى المسموح)
};

// بتحصر القيمة في المدى المسموح، وتقرر اتجاه IN1/IN2 حسب الإشارة (موجب/سالب)
MotorDriveSignal compute_motor_drive_signal(int pwm, int max_magnitude);

#endif