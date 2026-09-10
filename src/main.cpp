#include <Arduino.h>
#include "robot_config.h"
#include "robot_hal.h"
#include "hw_motors.h"
#include "hw_sensors.h"
#include "hw_safety.h"
#include "logic_wall_detection.h"

extern "C" {
    #include "maze_solver.h"
}

// إحداثيات المركز في متاهة 16x16
stPosition center_targets[] = {{7,7}, {7,8}, {8,7}, {8,8}};

// الحالة الحالية للروبوت
stPosition current_pos = {0, 0};
enDirection current_dir = DIR_NORTH;

// ==========================================================
// دوال الحركة التنفيذية (The Actuators)
// ==========================================================

void execute_forward_step() {
    encoder_reset();
    // التقدم مسافة خلية كاملة (180 مم) مع ترك هامش بسيط للتوقف (مثلاً 175 مم)
    while (motors_get_left_distance_mm() < 175.0f) {
        if (safety_is_stopped()) { motors_stop(); while(1); } // حماية الطوارئ
        
        // سرعة ثابتة 150 مم/ث لكلتا العجلتين
        motors_set_speed_mm_s(150.0f, 150.0f, 0.02f); 
        delay(20);
    }
    motors_stop();
    delay(100); // استقرار ميكانيكي قبل القراءة القادمة
}

void execute_turn(enDirection target_dir) {
    if (current_dir == target_dir) return; // لا حاجة للدوران

    int diff = (target_dir - current_dir + 4) % 4;
    float target_angle = 0.0f;
    
    if (diff == 1) target_angle = -90.0f;      // يمين
    else if (diff == 3) target_angle = 90.0f;  // يسار
    else if (diff == 2) target_angle = 180.0f; // للخلف (U-Turn)

    sensors_reset_yaw(); // تصفير البوصلة
    
    // الدوران حتى نصل للزاوية المطلوبة (باستخدام IMU)
    while (abs(sensors_get_yaw()) < abs(target_angle) - 2.0f) { // خصم درجتين للقصور الذاتي
        if (safety_is_stopped()) { motors_stop(); while(1); }
        
        sensors_update_yaw(0.02f); // dt = 20ms
        
        if (target_angle < 0) { // يمين
            motors_set_speed_mm_s(100.0f, -100.0f, 0.02f);
        } else { // يسار أو U-turn
            motors_set_speed_mm_s(-100.0f, 100.0f, 0.02f);
        }
        delay(20);
    }
    motors_stop();
    current_dir = target_dir;
    delay(100);
}

// دالة مساعدة لتحديث الإحداثيات بعد الحركة
void update_position_coordinates() {
    switch (current_dir) {
        case DIR_NORTH: current_pos.y++; break;
        case DIR_EAST:  current_pos.x++; break;
        case DIR_SOUTH: current_pos.y--; break;
        case DIR_WEST:  current_pos.x--; break;
    }
}

// ==========================================================
// العقل المدبر (The Main Loop)
// ==========================================================

void setup() {
    HAL_init();
    safety_init();
    motors_init();
    sensors_init();
    maze_init();

    Serial.println("System Ready. Waiting for Launch Button...");

    // انتظار ضغطة زر البدء
    while (!HAL_is_launch_button_pressed()) {
        delay(50);
    }

    uint8_t mode = HAL_get_dip_switch_mode();
    Serial.print("Starting Mode: ");
    Serial.println(mode);

    if (mode == 0) {
        // ---------------------------------------------------
        // المرحلة الأولى: وضع الاستكشاف (Search Run)
        // ---------------------------------------------------
        bool reached_center = false;
        
        while (!reached_center) {
            // 1. فحص الوصول للمركز
            for(int i=0; i<4; i++) {
                if(current_pos.x == center_targets[i].x && current_pos.y == center_targets[i].y) {
                    reached_center = true;
                }
            }
            if (reached_center) break;

            // 2. قراءة البيئة (الحساسات)
            IRReadings ir = sensors_read_ir();
            WallDetectionResult walls = detect_walls(current_dir, ir.front, ir.left, ir.right, IR_WALL_THRESHOLD_FRONT, IR_WALL_THRESHOLD_SIDE);

            // 3. تحديث الخريطة (كود محمد)
            maze_update_wall(current_pos.x, current_pos.y, walls.front_dir, walls.front_wall);
            maze_update_wall(current_pos.x, current_pos.y, walls.left_dir, walls.left_wall);
            maze_update_wall(current_pos.x, current_pos.y, walls.right_dir, walls.right_wall);

            // 4. الحساب الاستراتيجي
            flood_fill_recalculate(center_targets, 4);

            // 5. اتخاذ القرار والتنفيذ
            enDirection next_move = maze_get_next_move(current_pos, current_dir);
            execute_turn(next_move);
            execute_forward_step();
            update_position_coordinates();
        }

        // بمجرد الوصول، نحفظ المصفوفة في الذاكرة الدائمة
        HAL_save_data_to_flash("maze_map", (uint8_t*)maze, sizeof(maze));
        Serial.println("Maze mapped and saved to flash!");

    } else if (mode == 1) {
        // ---------------------------------------------------
        // المرحلة الثانية: وضع الانطلاق السريع (Speed Run)
        // ---------------------------------------------------
        if (HAL_load_data_from_flash("maze_map", (uint8_t*)maze, sizeof(maze))) {
            bool reached_center = false;
            
            while (!reached_center) {
                // فحص المركز
                for(int i=0; i<4; i++) {
                    if(current_pos.x == center_targets[i].x && current_pos.y == center_targets[i].y) {
                        reached_center = true;
                    }
                }
                if (reached_center) break;

                // هنا لا نقرأ الحساسات الجانبية لاكتشاف الجدران، نعتمد على الخريطة المحفوظة
                flood_fill_recalculate(center_targets, 4);
                enDirection next_move = maze_get_next_move(current_pos, current_dir);
                
                execute_turn(next_move);
                
                // يمكننا زيادة السرعة هنا لأن الخريطة آمنة
                execute_forward_step(); 
                update_position_coordinates();
            }
            Serial.println("Speed Run Completed!");
        } else {
            Serial.println("Error: No maze data in flash!");
        }
    }
}

void loop() {
    // إيقاف المحركات تماماً بعد انتهاء المهمة
    motors_stop();
}