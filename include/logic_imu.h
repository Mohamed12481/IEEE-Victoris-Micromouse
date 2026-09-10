#ifndef IMU_MATH_H
#define IMU_MATH_H

// ==========================================================
// معادلات تحويل قراءات IMU الخام لوحدات فيزيائية - مستقلة تمامًا
// عن أي اتصال هاردوير (I2C)، عشان تتاختبر native من غير شريحة حقيقية
// ==========================================================

// تحويل رقم خام من الأكسلرومتر لوحدة g (حسب حساسية المدى المستخدم)
float imu_raw_to_g(int raw, float lsb_per_g);

// تحويل رقم خام من الجيروسكوب لوحدة درجة/ثانية (deg/s)
float imu_raw_to_dps(int raw, float lsb_per_dps);

// تكامل الـ yaw: الزاوية الجديدة = القديمة + (سرعة الدوران × الوقت)
float integrate_yaw(float current_yaw, float gyro_z_dps, float dt);

// دمج بايتين (عالي ومنخفض) في رقم واحد 16-بت موقّع (signed)
int imu_combine_bytes(unsigned char high_byte, unsigned char low_byte);

#endif // IMU_MATH_H