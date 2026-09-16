// ===== FILE: src/motion_control.cpp =====
#include "robot_config.h"
#include <Arduino.h>
#include "hw_motors.h"
#include "hw_sensors.h"
#include "hw_safety.h"
#include "logic_wall_detection.h"
#include "maze_solver.h"

// ثوابت التحكم للتوسيط بين الحوائط (تم تعديلها مبدئياً)
#define KP_WALL 0.3f
#define KD_WALL 0.05f

static float last_wall_error = 0.0f;
extern enDirection current_dir; 

extern "C" {

void execute_forward_step() {
    encoder_reset();
    motors_reset_controllers(); // تصفير متحكمات السرعة لتجنب قفزات البداية
    last_wall_error = 0.0f;
    
    unsigned long last_time = micros(); // استخدام الميكروثانية لدقة الـ dt
    
    // التقدم لمسافة الخلية (175 مم لترك هامش للقصور الذاتي)
    while (motors_get_left_distance_mm() < 175.0f) {
        if (safety_is_stopped()) { motors_stop(); return; }
        
        // حساب الزمن الفعلي الدقيق للـ PID
        unsigned long current_time = micros();
        float dt = (current_time - last_time) / 1000000.0f; 
        if (dt <= 0.0001f) dt = 0.001f; // حماية رياضية
        last_time = current_time;
        
        // قراءة الحساسات وحساب خطأ التوسيط
        IRReadings ir = sensors_read_ir();
        float error = 0.0f;
        float delta_u = 0.0f;
        
        bool has_left = is_wall_detected(ir.left, IR_WALL_THRESHOLD_SIDE);
        bool has_right = is_wall_detected(ir.right, IR_WALL_THRESHOLD_SIDE);

        // التوسيط يعمل فقط إذا كان هناك حائطين للاعتماد عليهما
        if (has_left && has_right) {
            error = (float)(ir.left - ir.right);
            // المعادلة الهندسية الصحيحة: التفاضل يُقسم على الزمن
            float derivative = (error - last_wall_error) / dt;
            delta_u = (KP_WALL * error) + (KD_WALL * derivative);
            last_wall_error = error;
        } else {
            // تصفير الخطأ في حال غياب حائط لمنع الانحراف المفاجئ
            last_wall_error = 0.0f; 
        }

        // تطبيق السرعة الأساسية مضافاً إليها التصحيح
        float base_speed = 150.0f; 
        motors_set_speed_mm_s(base_speed - delta_u, base_speed + delta_u, dt);
        
        // الحماية من الاصطدام الأمامي أثناء السرعة
        if (ir.front > IR_CRITICAL_THRESHOLD) break;
        
        delay(2); // تفريغ المعالج لتجنب تعليق نظام الـ Watchdog
    }
    
    motors_stop();
    delay(100); // استقرار ميكانيكي قبل اتخاذ القرار التالي
}

void execute_turn(enDirection target_dir) {
    if (current_dir == target_dir) return;

    int diff = (target_dir - current_dir + 4) % 4;
    float target_angle = 0.0f;
    
    if (diff == 1) target_angle = -90.0f;      // يمين
    else if (diff == 3) target_angle = 90.0f;  // يسار
    else if (diff == 2) target_angle = 180.0f; // للخلف (U-Turn)

    sensors_reset_yaw(); 
    motors_reset_controllers(); // تصفير لتجنب الاهتزاز
    unsigned long last_time = micros();
    
    // الدوران مع خصم درجتين لتعويض القصور الذاتي الميكانيكي
    while (abs(sensors_get_yaw()) < abs(target_angle) - 2.0f) {
        if (safety_is_stopped()) { motors_stop(); return; }
        
        unsigned long current_time = micros();
        float dt = (current_time - last_time) / 1000000.0f;
        if (dt <= 0.0001f) dt = 0.001f;
        last_time = current_time;
        
        sensors_update_yaw(dt); 
        
        if (target_angle < 0) {
            motors_set_speed_mm_s(120.0f, -120.0f, dt); // سرعة دوران معدلة
        } else {
            motors_set_speed_mm_s(-120.0f, 120.0f, dt);
        }
        delay(2);
    }
    
    motors_stop();
    current_dir = target_dir;
    delay(150); // إعطاء IMU فرصة للاستقرار
}

} // end extern "C"