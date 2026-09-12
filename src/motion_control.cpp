#include "robot_config.h"
#include <Arduino.h>
#include "hw_motors.h"
#include "logic_wall_detection.h"

// الثوابت الخاصة بالتحكم (يتم ضبطها في المعمل)
#define KP_WALL 0.5
#define KD_WALL 0.1

int last_error = 0;

extern "C" {

void move_forward_cell() {
    encoder_reset(); // تم التعديل لتطابق hw_motors.h
    int base_speed = 150;
    
    // تم التعديل لاستخدام دوال جلب الـ Ticks بدلاً من المتغيرات المباشرة
    while ((encoder_get_left_ticks() + encoder_get_right_ticks()) / 2 < TICKS_PER_CELL) {
        
        // 1. قراءة الحساسات الجانبية
        int left_dist = analogRead(IR_LEFT_RX_PIN);
        int right_dist = analogRead(IR_RIGHT_RX_PIN);
        
        int error = 0;
        int delta_u = 0;
        
        // 2. حساب خطأ التوسيط باستخدام دالة is_wall_detected المتاحة لديكم
        if (is_wall_detected(left_dist, IR_WALL_THRESHOLD_SIDE) && 
            is_wall_detected(right_dist, IR_WALL_THRESHOLD_SIDE)) {
            
            error = left_dist - right_dist;
            delta_u = (KP_WALL * error) + (KD_WALL * (error - last_error));
            last_error = error;
        }

        // 3. تطبيق التصحيح على المواتير
        int left_speed = base_speed - delta_u;
        int right_speed = base_speed + delta_u;
        
        motors_set_pwm(left_speed, right_speed);
        
        // 4. حماية من الاصطدام الأمامي
        if (analogRead(IR_FRONT_RX_PIN) > IR_CRITICAL_THRESHOLD) {
            break; 
        }
    }
    motors_stop();
}

void turn_robot(float angle_degrees) {
    encoder_reset(); 
    
    // حساب الـ Ticks المطلوبة للدوران
    float angle_rad = angle_degrees * (PI / 180.0);
    float arc_length = angle_rad * (WHEEL_BASE_MM / 2.0);
    long target_ticks = arc_length / D_TICKS; 
    
    int turn_speed = 120;
    
    if (angle_degrees > 0) {
        // دوران لليسار (مثلاً 90 درجة)
        while (encoder_get_right_ticks() < target_ticks) {
            motors_set_pwm(-turn_speed, turn_speed);
        }
    } else {
        // دوران لليمين (مثلاً -90 درجة)
        while (encoder_get_left_ticks() < target_ticks) {
            motors_set_pwm(turn_speed, -turn_speed);
        }
    }
    motors_stop();
}

} // end extern "C"