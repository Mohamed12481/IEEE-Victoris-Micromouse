#ifndef ENCODER_MATH_H
#define ENCODER_MATH_H

// ==========================================================
// معادلات الإنكودر الرياضية - مستقلة تمامًا عن أي هاردوير
// عشان تتاختبر native من غير إنكودر حقيقي متوصل
// ==========================================================

// حساب المسافة اللي بتمثلها نبضة واحدة (tick) بالمليمتر
// = محيط العجلة ÷ عدد النبضات في الدورة الكاملة
float compute_distance_per_tick_mm(float wheel_diameter_mm, long ticks_per_rev);

// تحويل عدد نبضات لمسافة بالمليمتر (ممكن يكون سالب لو اتجاه عكسي)
float ticks_to_distance_mm(long ticks, float distance_per_tick_mm);

// تحديد اتجاه الدوران من حالة قناة B وقت ما قناة A بتطلع (quadrature decoding)
// بيرجع +1 لو قدام (b_state = true/HIGH)، -1 لو ورا (b_state = false/LOW)
int encoder_direction_from_b_state(bool b_state);

#endif // ENCODER_MATH_H