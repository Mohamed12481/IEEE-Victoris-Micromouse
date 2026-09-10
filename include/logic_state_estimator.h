// #ifndef STATE_ESTIMATOR_H
// #define STATE_ESTIMATOR_H

// #ifdef ARDUINO
// #include <Arduino.h>
// #else
// #include <cstdint>
// #endif
// #include "maze_solver.h"   // بنستخدم نفس enum Direction بتاع البوصلة

// // ==========================================================
// // حالة الروبوت الحالية: في أنهي خلية وواقف في أنهي اتجاه بوصلة
// // ==========================================================
// struct RobotState {
//     uint8_t   cell_x;
//     uint8_t   cell_y;
//     Direction heading;   // الاتجاه اللي الروبوت شايفه دلوقتي (بوصلة)
// };

// // الحركة المطلوبة بالنسبة للروبوت نفسه (مش بالنسبة للبوصلة)
// enum RobotAction {
//     ACTION_FORWARD,
//     ACTION_TURN_LEFT,
//     ACTION_TURN_RIGHT,
//     ACTION_TURN_AROUND,
//     ACTION_NONE
// };

// // ==========================================================
// // التهيئة - غالبًا خلية البداية (0,0) والاتجاه بتاع خلية البداية
// // ==========================================================
// void state_estimator_init(uint8_t start_x, uint8_t start_y, Direction start_heading);

// RobotState state_estimator_get_state();

// // حوّل اتجاه بوصلة (من maze_get_next_direction) لحركة روبوت (قدام/لف يمين/شمال/دوران)
// RobotAction state_estimator_get_action_for_direction(Direction target_dir);

// // حدّث حالة الروبوت بعد تنفيذ حركة فعليًا (استدعيها بعد كل خطوة حركة ناجحة)
// void state_estimator_apply_action(RobotAction action);

// bool state_estimator_at_goal();

// // تحويل بين enum الاتجاه والـ index (0=شمال، 1=شرق، 2=جنوب، 3=غرب)
// // مفيدة لأي كود خارجي محتاج يحسب اتجاه نسبي (زي ربط قراءة حساس بالبوصلة)
// uint8_t direction_to_index(Direction dir);
// Direction index_to_direction(uint8_t index);

// #endif // STATE_ESTIMATOR_H