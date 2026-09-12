#include "hw_motors.h"
#include "robot_config.h"
#include "logic_encoder.h"
#include "logic_motors.h"

static PIDController pid_left;
static PIDController pid_right;

// ==========================================================
// إعدادات PWM (ESP32 LEDC) - عدّل القنوات لو بتستخدمها لحاجة تانية
// ==========================================================
#define PWM_FREQ_HZ     5000
#define PWM_RESOLUTION     8   // 0-255
#define PWM_CHANNEL_L      0
#define PWM_CHANNEL_R      1

static const float DISTANCE_PER_TICK_MM =
    compute_distance_per_tick_mm(WHEEL_DIAMETER_MM, ENCODER_TICKS_PER_REV);

// عدادات الإنكودر - لازم تتحدث جوه interrupt، فلازم تبقى volatile
volatile long g_encoder_left_ticks  = 0;
volatile long g_encoder_right_ticks = 0;

static long g_last_left_ticks  = 0;
static long g_last_right_ticks = 0;

// ==========================================================
// دوال الـ ISR - بتتنفذ عند كل نبضة من قناة A لكل إنكودر
// ==========================================================
void IRAM_ATTR isr_encoder_left() {
    bool b_state = digitalRead(ENCODER_L_B_PIN);
    g_encoder_left_ticks += encoder_direction_from_b_state(b_state);
}

void IRAM_ATTR isr_encoder_right() {
    bool b_state = digitalRead(ENCODER_R_B_PIN);
    g_encoder_right_ticks += encoder_direction_from_b_state(b_state);
}


// ==========================================================
// التهيئة
// ==========================================================
void motors_init() {
    // تهيئة الـ PID للسرعة (مع تحديد الحد التكاملي 200.0)
    pid_left = pid_create(SPEED_KP, SPEED_KI, SPEED_KD, 200.0);
    pid_right = pid_create(SPEED_KP, SPEED_KI, SPEED_KD, 200.0);

    pinMode(MOTOR_L_IN1_PIN, OUTPUT);
    pinMode(MOTOR_L_IN2_PIN, OUTPUT);
    pinMode(MOTOR_R_IN1_PIN, OUTPUT);
    pinMode(MOTOR_R_IN2_PIN, OUTPUT);
    pinMode(MOTOR_STBY_PIN, OUTPUT);
    digitalWrite(MOTOR_STBY_PIN, HIGH); // خروج من وضع الـ standby

    ledcSetup(PWM_CHANNEL_L, PWM_FREQ_HZ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_R, PWM_FREQ_HZ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_L_PWM_PIN, PWM_CHANNEL_L);
    ledcAttachPin(MOTOR_R_PWM_PIN, PWM_CHANNEL_R);

    pinMode(ENCODER_L_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_L_B_PIN, INPUT_PULLUP);
    pinMode(ENCODER_R_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_R_B_PIN, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(ENCODER_L_A_PIN), isr_encoder_left, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_R_A_PIN), isr_encoder_right, RISING);

    motors_stop();
}

// ==========================================================
// تحكم مباشر بالـ PWM - القيمة السالبة تعني اتجاه عكسي
// ==========================================================
static void set_one_motor(int pwm, uint8_t in1_pin, uint8_t in2_pin, uint8_t channel) {
    MotorDriveSignal signal = compute_motor_drive_signal(pwm, 255);

    digitalWrite(in1_pin, signal.in1_high ? HIGH : LOW);
    digitalWrite(in2_pin, signal.in2_high ? HIGH : LOW);
    ledcWrite(channel, signal.pwm_magnitude);
}

void motors_set_pwm(int left_pwm, int right_pwm) {
    set_one_motor(left_pwm,  MOTOR_L_IN1_PIN, MOTOR_L_IN2_PIN, PWM_CHANNEL_L);
    set_one_motor(right_pwm, MOTOR_R_IN1_PIN, MOTOR_R_IN2_PIN, PWM_CHANNEL_R);
}

void motors_stop() {
    digitalWrite(MOTOR_L_IN1_PIN, LOW);
    digitalWrite(MOTOR_L_IN2_PIN, LOW);
    digitalWrite(MOTOR_R_IN1_PIN, LOW);
    digitalWrite(MOTOR_R_IN2_PIN, LOW);
    ledcWrite(PWM_CHANNEL_L, 0);
    ledcWrite(PWM_CHANNEL_R, 0);
}

// ==========================================================
// الإنكودر
// ==========================================================
void encoder_reset() {
    noInterrupts();
    g_encoder_left_ticks  = 0;
    g_encoder_right_ticks = 0;
    interrupts();
    g_last_left_ticks  = 0;
    g_last_right_ticks = 0;
}

long encoder_get_left_ticks()  { return g_encoder_left_ticks; }
long encoder_get_right_ticks() { return g_encoder_right_ticks; }

float motors_get_left_speed_mm_s(float dt) {
    long current = g_encoder_left_ticks;
    long delta = current - g_last_left_ticks;
    g_last_left_ticks = current;
    if (dt <= 0.0f) return 0.0f;
    return ticks_to_distance_mm(delta, DISTANCE_PER_TICK_MM) / dt;
}

float motors_get_right_speed_mm_s(float dt) {
    long current = g_encoder_right_ticks;
    long delta = current - g_last_right_ticks;
    g_last_right_ticks = current;
    if (dt <= 0.0f) return 0.0f;
    return ticks_to_distance_mm(delta, DISTANCE_PER_TICK_MM) / dt;
}

float motors_get_left_distance_mm() {
    return ticks_to_distance_mm(g_encoder_left_ticks, DISTANCE_PER_TICK_MM);
}

float motors_get_right_distance_mm() {
    return ticks_to_distance_mm(g_encoder_right_ticks, DISTANCE_PER_TICK_MM);
}

// ==========================================================
// تحكم سرعة closed-loop عن طريق PID
// ==========================================================
void motors_set_speed_mm_s(float left_target_mm_s, float right_target_mm_s, float dt) {
    // 1. حساب السرعة الفعلية الحالية
    float current_left_speed = motors_get_left_speed_mm_s(dt);
    float current_right_speed = motors_get_right_speed_mm_s(dt);

    // 2. حساب قيمة التصحيح من متحكم الـ PID
    float left_pwm = pid_compute(&pid_left, left_target_mm_s, current_left_speed, dt);
    float right_pwm = pid_compute(&pid_right, right_target_mm_s, current_right_speed, dt);

    // 3. التقليم (Saturation Clamping) لحماية مسجلات الهاردوير
    if (left_pwm > 255.0f) left_pwm = 255.0f;
    if (left_pwm < -255.0f) left_pwm = -255.0f;
    if (right_pwm > 255.0f) right_pwm = 255.0f;
    if (right_pwm < -255.0f) right_pwm = -255.0f;

    // 4. إرسال الطاقة للمحركات
    motors_set_pwm((int)left_pwm, (int)right_pwm);
}
