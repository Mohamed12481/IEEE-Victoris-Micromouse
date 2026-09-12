#include "robot_config.h"
#include <Arduino.h>
#include "hw_motors.h"
#include "hw_sensors.h"
#include "hw_safety.h"
#include "logic_wall_detection.h"
#include "maze_solver.h"

// ثوابت التحكم للتوسيط بين الحوائط
#define KP_WALL 0.5
#define KD_WALL 0.1

static int last_error = 0;
extern enDirection current_dir; // جلب الاتجاه الحالي من main.cpp

extern "C" {

void execute_forward_step() {
    encoder_reset();
    unsigned long last_time = millis();
    
    // التقدم لمسافة 175 مم (ترك هامش للقصور الذاتي)
    while (motors_get_left_distance_mm() < 175.0f) {
        if (safety_is_stopped()) { motors_stop(); while(1); }
        
        // حساب الزمن الفعلي الدقيق للـ PID
        unsigned long current_time = millis();
        float dt = (current_time - last_time) / 1000.0f;
        if (dt <= 0.0f) dt = 0.001f; 
        last_time = current_time;
        
        // قراءة الحساسات وحساب خطأ التوسيط
        IRReadings ir = sensors_read_ir();
        int error = 0;
        float delta_u = 0.0f;
        
        if (is_wall_detected(ir.left, IR_WALL_THRESHOLD_SIDE) && 
            is_wall_detected(ir.right, IR_WALL_THRESHOLD_SIDE)) {
            error = ir.left - ir.right;
            delta_u = (KP_WALL * error) + (KD_WALL * (error - last_error));
            last_error = error;
        }

        // تطبيق السرعة الأساسية مضافاً إليها التصحيح
        float base_speed = 150.0f; 
        motors_set_speed_mm_s(base_speed - delta_u, base_speed + delta_u, dt);
        
        // الحماية من الاصطدام الأمامي أثناء السرعة
        if (ir.front > IR_CRITICAL_THRESHOLD) break;
        
        delay(5); // تفريغ المعالج لتجنب تعليق نظام الـ Watchdog
    }
    motors_stop();
    delay(100); // استقرار ميكانيكي قبل الخلية التالية
}

void execute_turn(enDirection target_dir) {
    if (current_dir == target_dir) return;

    int diff = (target_dir - current_dir + 4) % 4;
    float target_angle = 0.0f;
    
    if (diff == 1) target_angle = -90.0f;      // يمين
    else if (diff == 3) target_angle = 90.0f;  // يسار
    else if (diff == 2) target_angle = 180.0f; // للخلف (U-Turn)

    sensors_reset_yaw(); 
    unsigned long last_time = millis();
    
    // الدوران مع خصم درجتين لتعويض القصور الذاتي الميكانيكي
    while (abs(sensors_get_yaw()) < abs(target_angle) - 2.0f) {
        if (safety_is_stopped()) { motors_stop(); while(1); }
        
        unsigned long current_time = millis();
        float dt = (current_time - last_time) / 1000.0f;
        if (dt <= 0.0f) dt = 0.001f;
        last_time = current_time;
        
        sensors_update_yaw(dt); 
        
        if (target_angle < 0) {
            motors_set_speed_mm_s(100.0f, -100.0f, dt);
        } else {
            motors_set_speed_mm_s(-100.0f, 100.0f, dt);
        }
        delay(5);
    }
    motors_stop();
    current_dir = target_dir;
    delay(100);
}

} // end extern "C"