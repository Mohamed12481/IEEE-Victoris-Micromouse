#include "logic_encoder.h"

float compute_distance_per_tick_mm(float wheel_diameter_mm, long ticks_per_rev) {
    float circumference = 3.14159265359f * wheel_diameter_mm; // محيط العجلة = π * d
    return circumference / (float)ticks_per_rev;
}

float ticks_to_distance_mm(long ticks, float distance_per_tick_mm) {
    return (float)ticks * distance_per_tick_mm;
}

int encoder_direction_from_b_state(bool b_state) {
    // مبدأ الـ Quadrature Decoding: قناة B تحدد الاتجاه
    return b_state ? 1 : -1; 
}