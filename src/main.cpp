// ===== FILE: src/main.cpp (Autonomous Micromouse 16x16 - Competition Ready) =====

#include <Arduino.h>
#include "robot_config.h"
#include "robot_hal.h"
#include "hw_motors.h"
#include "hw_sensors.h"
#include "maze_solver.h"

// ==========================================================
// 1. المتغيرات العامة للموقع والاتجاه
// ==========================================================
stPosition current_pos = {0, 0};
enDirection current_dir = DIR_NORTH;

// خلايا المنتصف الأربع المحددة لهدف المتاهة 16x16
stPosition goal_nodes[4] = {
    {7, 7}, {7, 8}, {8, 7}, {8, 8}
};

// ==========================================================
// 2. دوال الحركة الميكانيكية بالـ Encoders
// ==========================================================

// فرملة نشطة مشدودة لشريحة TB6612FNG لامتصاص القصور الذاتي للبطارية
void motors_brake() {
    digitalWrite(MOTOR_L_IN1_PIN, HIGH);
    digitalWrite(MOTOR_L_IN2_PIN, HIGH);
    digitalWrite(MOTOR_R_IN1_PIN, HIGH);
    digitalWrite(MOTOR_R_IN2_PIN, HIGH);
    ledcWrite(0, 255);
    ledcWrite(1, 255);
}

// التقدم خلية واحدة 180 مم بخصم هامش القصور الذاتي (13 مم)
void execute_forward_step(int pwm_speed = 135, float brake_margin_mm = 13.0f) {
    encoder_reset();

    motors_set_pwm(pwm_speed, pwm_speed);
    float target_mm = 180.0f - brake_margin_mm;

    while (motors_get_left_distance_mm() < target_mm) {
        delay(2);
    }

    motors_brake();
    delay(60);
    motors_stop();
    delay(150); // مهلة استقرار ميكانيكي قصيرة
}

// الدوران بالـ Encoders بناءً على أبعاد العجلتين (10 سم)
void execute_turn_encoder(bool is_clockwise, float angle_deg, int pwm_speed = 120, float brake_margin_ticks = 18.0f) {
    encoder_reset();

    float ticks_per_mm = (619.0f / 180.0f); // ~3.4388 Ticks/mm
    float turn_arc_90_mm = 78.54f;          // (PI / 2) * 50mm
    float ticks_90_deg = turn_arc_90_mm * ticks_per_mm; // ~270 Ticks

    float total_target_ticks = (angle_deg / 90.0f) * ticks_90_deg;
    float target_ticks = total_target_ticks - brake_margin_ticks;

    if (is_clockwise) {
        motors_set_pwm(pwm_speed, -pwm_speed); // يمين
    } else {
        motors_set_pwm(-pwm_speed, pwm_speed); // يسار
    }

    while (abs(encoder_get_left_ticks()) < target_ticks) {
        delay(2);
    }

    motors_brake();
    delay(60);
    motors_stop();
    delay(150);
}

// ==========================================================
// 3. قراءة الحساسات وتحديث حوائط الخوارزمية
// ==========================================================
void read_walls_and_update_maze() {
    IRReadings ir = sensors_read_ir();

    // مقارنة القراءات الصافية بـ thresholds المعايرة
    bool wall_front = (ir.front > IR_WALL_THRESHOLD_FRONT);
    bool wall_left  = (ir.left  > IR_WALL_THRESHOLD_SIDE);
    bool wall_right = (ir.right > IR_WALL_THRESHOLD_SIDE);

    // تحديث الحائط الأمامي
    maze_update_wall(current_pos.x, current_pos.y, current_dir, wall_front);

    // تحويل الاتجاهات النسبية إلى اتجاهات جغرافية مطلقة
    enDirection right_dir = (enDirection)((current_dir + 1) % 4);
    enDirection left_dir  = (enDirection)((current_dir + 3) % 4);

    maze_update_wall(current_pos.x, current_pos.y, right_dir, wall_right);
    maze_update_wall(current_pos.x, current_pos.y, left_dir, wall_left);
}

// ==========================================================
// 4. توجيه الروبوت فيزيائياً للاتجاه التالي المختار
// ==========================================================
void move_robot_to_direction(enDirection next_dir) {
    int diff = (next_dir - current_dir + 4) % 4;

    if (diff == 1) {
        // دوران 90 درجة يميناً
        execute_turn_encoder(true, 90.0f, 120, 18.0f);
    } else if (diff == 3) {
        // دوران 90 درجة يساراً
        execute_turn_encoder(false, 90.0f, 120, 18.0f);
    } else if (diff == 2) {
        // دوران 180 درجة (U-Turn)
        execute_turn_encoder(true, 180.0f, 120, 25.0f);
    }

    // تحديث اتجاه الروبوت الحالي
    current_dir = next_dir;

    // التقدم خلية واحدة 180 مم
    execute_forward_step(135, 13.0f);

    // تحديث إحداثيات الموقع
    switch (current_dir) {
        case DIR_NORTH: current_pos.y++; break;
        case DIR_EAST:  current_pos.x++; break;
        case DIR_SOUTH: current_pos.y--; break;
        case DIR_WEST:  current_pos.x--; break;
    }
}

// ==========================================================
// 5. التهيئة Setup
// ==========================================================
void setup() {
    Serial.begin(115200);

    // تهيئة الهاردوير والمحركات والمستشعرات
    HAL_init();
    motors_init();
    sensors_init();
    motors_stop();

    // تهيئة المتاهة وخوارزمية Flood Fill لمتاهة 16x16
    maze_init();
    flood_fill_recalculate(goal_nodes, 4);

    Serial.println("\n=============================================");
    Serial.println("  AUTONOMOUS MICROMOUSE READY - STARTING IN 3S ");
    Serial.println("=============================================");

    // مهلة 3 ثوانٍ لوضع الروبوت في خلية البداية (0,0) وسحب اليد
    delay(3000); 
}

// ==========================================================
// 6. حلقة التحكم الرئيسية Loop
// ==========================================================
void loop() {
    // 1. التحقق من الوصول لأحد خلايا الهدف الأربع (Distance == 0)
    if (maze[current_pos.x][current_pos.y].distance == 0) {
        motors_stop();
        Serial.println("\n[SUCCESS] TARGET GOAL REACHED!");

        // التوقف النهائي لحسم المسابقة من المحاولة الأولى
        while (true) {
            motors_stop();
            delay(1000);
        }
    }

    // 2. قراءة الحساسات وتحديث خريطة الحوائط
    read_walls_and_update_maze();

    // 3. إعادة حساب قيم المسافات لخوارزمية Flood Fill
    flood_fill_recalculate(goal_nodes, 4);

    // 4. اختيار الاتجاه القادم الأقل مسافة (مع ترجيح الأمام عند التساوي)
    enDirection next_dir = maze_get_next_move(current_pos, current_dir);

    // 5. تنفيذ الحركة الفيزيائية للخلية التالية
    move_robot_to_direction(next_dir);

    delay(100); // مهلة استقرار ميكانيكي بين الخلايا
}