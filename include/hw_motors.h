// ===== FILE: include/hw_motors.h =====
#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "logic_pid.h"

// ==========================================================
// التهيئة
// ==========================================================
void motors_init();

// ==========================================================
// تحكم مباشر في الـ PWM (من -255 لـ 255، السالب = رجوع للخلف)
// ==========================================================
void motors_set_pwm(int left_pwm, int right_pwm);
void motors_stop();

// ==========================================================
// الإنكودر
// ==========================================================
void encoder_reset();
long encoder_get_left_ticks();
long encoder_get_right_ticks();

// السرعة الحالية بالـ mm/s
float motors_get_left_speed_mm_s(float dt);
float motors_get_right_speed_mm_s(float dt);

// المسافة الكلية المقطوعة من آخر encoder_reset() بالمليمتر
float motors_get_left_distance_mm();
float motors_get_right_distance_mm();

// ==========================================================
// تحكم بالسرعة المستهدفة (closed loop عن طريق PID)
// ==========================================================
void motors_set_speed_mm_s(float left_target_mm_s, float right_target_mm_s, float dt);

// إضافة جديدة: تصفير متحكمات السرعة قبل كل حركة جديدة
void motors_reset_controllers();

#endif // MOTOR_CONTROL_H