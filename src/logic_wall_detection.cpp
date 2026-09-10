#include "logic_wall_detection.h"

bool is_wall_detected(int ir_reading, int threshold) {
    return ir_reading > threshold;
}

WallDetectionResult detect_walls(
    enDirection heading,
    int ir_front, int ir_left, int ir_right,
    int threshold_front, int threshold_side
) {
    WallDetectionResult result;

    // الاعتماد المباشر على قيمة الاتجاه الحالي دون دوال وسيطة
    result.front_dir = heading;
    result.left_dir  = (enDirection)((heading + 3) % 4);
    result.right_dir = (enDirection)((heading + 1) % 4);

    result.front_wall = is_wall_detected(ir_front, threshold_front);
    result.left_wall  = is_wall_detected(ir_left,  threshold_side);
    result.right_wall = is_wall_detected(ir_right, threshold_side);

    return result;
}