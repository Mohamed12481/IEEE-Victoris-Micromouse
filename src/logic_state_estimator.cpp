// #include "logic_state_estimator.h"

// static RobotState g_state;

// // ==========================================================
// // تحويل بين enum الاتجاه والـ index (0=شمال، 1=شرق، 2=جنوب، 3=غرب)
// // عشان نقدر نحسب الفرق بين اتجاهين بسهولة
// // ==========================================================
// uint8_t direction_to_index(Direction dir) {
//     switch (dir) {
//         case DIR_NORTH: return 0;
//         case DIR_EAST:  return 1;
//         case DIR_SOUTH: return 2;
//         case DIR_WEST:  return 3;
//     }
//     return 0;
// }

// Direction index_to_direction(uint8_t index) {
//     static const Direction table[4] = { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };
//     return table[index % 4];
// }

// // ==========================================================
// // التهيئة
// // ==========================================================
// void state_estimator_init(uint8_t start_x, uint8_t start_y, Direction start_heading) {
//     g_state.cell_x = start_x;
//     g_state.cell_y = start_y;
//     g_state.heading = start_heading;
// }

// RobotState state_estimator_get_state() {
//     return g_state;
// }

// // ==========================================================
// // تحويل اتجاه البوصلة المطلوب لحركة روبوت (بالنسبة لاتجاهه الحالي)
// // ==========================================================
// RobotAction state_estimator_get_action_for_direction(Direction target_dir) {
//     uint8_t current_index = direction_to_index(g_state.heading);
//     uint8_t target_index  = direction_to_index(target_dir);

//     int8_t diff = (int8_t)target_index - (int8_t)current_index;
//     diff = ((diff % 4) + 4) % 4;   // نتأكد إنه موجب بين 0 و 3

//     switch (diff) {
//         case 0: return ACTION_FORWARD;      // نفس الاتجاه
//         case 1: return ACTION_TURN_RIGHT;   // لف يمين ربع دورة
//         case 2: return ACTION_TURN_AROUND;  // دوران كامل (نص دورة)
//         case 3: return ACTION_TURN_LEFT;    // لف شمال ربع دورة
//     }
//     return ACTION_NONE;
// }

// // ==========================================================
// // تحديث حالة الروبوت بعد تنفيذ الحركة فعليًا
// // ==========================================================
// void state_estimator_apply_action(RobotAction action) {
//     uint8_t current_index = direction_to_index(g_state.heading);

//     switch (action) {
//         case ACTION_TURN_LEFT:
//             g_state.heading = index_to_direction((current_index + 3) % 4); // -90
//             break;

//         case ACTION_TURN_RIGHT:
//             g_state.heading = index_to_direction((current_index + 1) % 4); // +90
//             break;

//         case ACTION_TURN_AROUND:
//             g_state.heading = index_to_direction((current_index + 2) % 4); // +180
//             break;

//         case ACTION_FORWARD:
//             // الروبوت اتحرك خلية واحدة قدام - حدّث الإحداثيات حسب اتجاهه
//             switch (g_state.heading) {
//                 case DIR_NORTH: g_state.cell_y++; break;
//                 case DIR_SOUTH: g_state.cell_y--; break;
//                 case DIR_EAST:  g_state.cell_x++; break;
//                 case DIR_WEST:  g_state.cell_x--; break;
//             }
//             break;

//         case ACTION_NONE:
//         default:
//             break;
//     }
// }

// bool state_estimator_at_goal() {
//     return maze_is_goal_cell(g_state.cell_x, g_state.cell_y);
// }