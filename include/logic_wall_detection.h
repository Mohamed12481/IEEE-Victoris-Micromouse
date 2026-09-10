#ifndef WALL_DETECTION_H
#define WALL_DETECTION_H

#include "maze_solver.h"  // الاعتماد على هيكلة الاتجاهات القياسية للخوارزمية

// ==========================================================
// نتيجة اكتشاف الحيطان: أي الحساسات شافت حيطة، وفي أنهي اتجاه بوصلة
// ==========================================================
struct WallDetectionResult {
    bool front_wall;
    bool left_wall;
    bool right_wall;
    enDirection front_dir; // تم التعديل
    enDirection left_dir;  // تم التعديل
    enDirection right_dir; // تم التعديل
};

bool is_wall_detected(int ir_reading, int threshold);

WallDetectionResult detect_walls(
    enDirection heading, // تم التعديل
    int ir_front, int ir_left, int ir_right,
    int threshold_front, int threshold_side
);

#endif // WALL_DETECTION_H