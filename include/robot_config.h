#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

// ==========================================================
// أبعاد المتاهة - حسب قواعد VICTORIS 5.0
// ==========================================================
#define MAZE_SIZE           16      // 16 x 16 خانة
#define CELL_SIZE_MM        180     // كل خانة 18 سم x 18 سم
#define WALL_THICKNESS_MM   12      // سمك الحيطة 1.2 سم
#define WALL_HEIGHT_MM      50      // ارتفاع الحيطة 5 سم
#define MAZE_TOTAL_SIZE_MM  2880    // 288 سم x 288 سم تقريباً

// ==========================================================
// حدود الروبوت (قيود المسابقة)
// ==========================================================
#define ROBOT_MAX_WIDTH_MM   140    
#define ROBOT_MAX_LENGTH_MM  140    
#define ROBOT_MAX_WEIGHT_G  1500    
#define MAX_VOLTAGE_V         12    

// ==========================================================
// أبعاد الروبوت الفعلية والحسابات المسبقة
// ==========================================================
#define WHEEL_DIAMETER_MM      34.0   
#define WHEEL_BASE_MM          100.0  
#define ENCODER_TICKS_PER_REV  210.0  // (7 PPR * 30 Gear Ratio)
#define D_TICKS                0.508  
#define TICKS_PER_CELL         354    

// ==========================================================
// بينات الموتور - TB6612FNG (مطابقة 100% للسكيماتيك)
// ==========================================================
#define MOTOR_L_PWM_PIN   23  // PWMA
#define MOTOR_L_IN1_PIN   26  // AIN1
#define MOTOR_L_IN2_PIN   25  // AIN2

#define MOTOR_R_PWM_PIN   13  // PWMB
#define MOTOR_R_IN1_PIN   27  // BIN1 (تم التصحيح حسب السكيماتيك)
#define MOTOR_R_IN2_PIN   14  // BIN2 (تم التصحيح حسب السكيماتيك)

// ==========================================================
// إنكودر العجلتين
// ==========================================================
#define ENCODER_L_A_PIN   16  // RX2
#define ENCODER_L_B_PIN   17  // TX2
#define ENCODER_R_A_PIN   18  // SCK
#define ENCODER_R_B_PIN   19  // MISO

// ==========================================================
// بينات حساسات الـ IR
// ==========================================================
#define IR_EMITTERS_PIN   4   // بوابة الـ MOSFET

#define IR_RIGHT_RX_PIN   32  
#define IR_DIAG_L_RX_PIN  33
#define IR_LEFT_RX_PIN    34  
#define IR_FRONT_RX_PIN   35  
#define IR_DIAG_R_RX_PIN  36

// ==========================================================
// بينات الـ IMU - BMI160
// ==========================================================
#define IMU_SDA_PIN    21
#define IMU_SCL_PIN    22
#define IMU_I2C_ADDR   0x69

// ==========================================================
// ثوابت PID - للـ Tuning في المعمل
// ==========================================================
#define SPEED_KP   1.5
#define SPEED_KI   0.0
#define SPEED_KD   0.1

#define HEADING_KP 0.5 
#define HEADING_KI 0.0
#define HEADING_KD 0.1

// ==========================================================
// حدود اكتشاف الحيطان 
// ==========================================================
#define IR_WALL_THRESHOLD_FRONT   500
#define IR_WALL_THRESHOLD_SIDE    400
#define IR_CRITICAL_THRESHOLD     800 

#endif