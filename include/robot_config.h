#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

// ==========================================================
// أبعاد المتاهة - حسب قواعد VICTORIS 5.0
// ==========================================================
#define MAZE_SIZE           16      // 16 x 16 خانة
#define CELL_SIZE_MM         180    // كل خانة 18 سم x 18 سم
#define WALL_THICKNESS_MM     12    // سمك الحيطة 1.2 سم
#define WALL_HEIGHT_MM        50    // ارتفاع الحيطة 5 سم
#define MAZE_TOTAL_SIZE_MM  2880    // 288 سم x 288 سم تقريبًا

// ==========================================================
// حدود الروبوت (قيود المسابقة)
// ==========================================================
#define ROBOT_MAX_WIDTH_MM   140    // أقصى عرض 14 سم
#define ROBOT_MAX_LENGTH_MM  140    // أقصى طول 14 سم
#define ROBOT_MAX_WEIGHT_G  1500    // أقصى وزن 1.5 كجم
#define MAX_VOLTAGE_V          12   // أقصى فولت مسموح 12V DC

// ==========================================================
// أبعاد الروبوت الفعلية (عدّل حسب تصميمك الفعلي)
// ==========================================================
#define WHEEL_DIAMETER_MM      32   // TODO: قيس قطر العجلة الفعلي
#define WHEEL_BASE_MM          80   // TODO: قيس المسافة بين مركزي العجلتين
#define ENCODER_TICKS_PER_REV 360   // TODO: عدّل حسب نوع الإنكودر

// ==========================================================
// بينات الموتور - TB6612FNG + JGA12-N20 (مؤكد من BOM)
// ==========================================================
#define MOTOR_L_PWM_PIN   25
#define MOTOR_L_IN1_PIN   26
#define MOTOR_L_IN2_PIN   27
#define MOTOR_R_PWM_PIN   14
#define MOTOR_R_IN1_PIN   15   // كانت 12 (strapping pin خطير)، اتغيرت لـ 15 (خطرها أقل بكتير)
#define MOTOR_R_IN2_PIN   13
#define MOTOR_STBY_PIN    33

// إنكودر العجلتين (quadrature - قناتين لكل موتور)
#define ENCODER_L_A_PIN   18
#define ENCODER_L_B_PIN   19
#define ENCODER_R_A_PIN   23
#define ENCODER_R_B_PIN    5

// ==========================================================
// بينات حساسات الـ IR - 3 حساسات (6 emitter + 6 receiver من BOM)
// كل حساس = emitter واحد + receiver واحد على نفس الاتجاه
// ==========================================================
#define IR_LEFT_RX_PIN    34   // ADC only - قراءة الـ receiver
#define IR_FRONT_RX_PIN   35   // ADC only - قراءة الـ receiver
#define IR_RIGHT_RX_PIN   32

// بينات تشغيل الـ emitters (تشغيل واحد واحد بالتبادل يقلل التداخل)
#define IR_LEFT_TX_PIN    16
#define IR_FRONT_TX_PIN   17
#define IR_RIGHT_TX_PIN   4

// ==========================================================
// بينات الـ IMU - I2C (MPU6500 / GY-6500 حسب BOM) على بينات ESP32 الافتراضية
// ==========================================================
#define IMU_SDA_PIN    21
#define IMU_SCL_PIN    22
#define IMU_I2C_ADDR   0x68

// ==========================================================
// زرار الطوارئ (Emergency Stop) - شرط إلزامي في قواعد المسابقة
// GPIO 39: بين "input only"، مفيهوش pull-up داخلي، فلازم مقاومة
// pull-up خارجية (10k) بين البين والـ 3.3V، والزرار نفسه بين
// البين والأرضي (يعني الزرار "active LOW": الضغط عليه = LOW)
// ==========================================================
#define EMERGENCY_STOP_PIN 39   // نفس عنوان MPU6050 غالبًا، تأكد من الداتا شيت

// ==========================================================
// ثوابت PID - قيم مبدئية، هنعمل tuning ليها بعدين
// ==========================================================
// PID التحكم في السرعة (لكل عجلة)
#define SPEED_KP   1.0
#define SPEED_KI   0.0
#define SPEED_KD   0.0

// PID التوازن / متابعة الحيطة (wall following / heading)
#define HEADING_KP 1.0
#define HEADING_KI 0.0
#define HEADING_KD 0.0

// ==========================================================
// حدود اكتشاف الحيطان من قراءات الـ IR - TODO: لازم تتظبط بالتجربة الفعلية
// ==========================================================
#define IR_WALL_THRESHOLD_FRONT   500
#define IR_WALL_THRESHOLD_SIDE    400

#endif // ROBOT_CONFIG_H