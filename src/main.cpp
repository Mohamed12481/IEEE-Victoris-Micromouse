#include <Arduino.h>

// ==========================================================
// دبابيس حساسات الـ IR مطابقة تماماً للـ PCB
// ==========================================================
#define IR_EMITTERS_PIN   4   // بوابة الـ MOSFET لتشغيل الباعثات

#define IR_LEFT_RX_PIN    34  // مستقبل يسار
#define IR_FRONT_RX_PIN   35  // مستقبل أمامي
#define IR_RIGHT_RX_PIN   32  // مستقبل يمين

// دالة قراءة الحساس مع تطبيق إلغاء الضوء المحيط (Ambient Light Cancellation)
int read_ir_sensor(uint8_t rx_pin, int &ambient_out, int &active_out) {
    // 1. قراءة الضوء المحيط والـ Emitter مطفي
    digitalWrite(IR_EMITTERS_PIN, LOW);
    delayMicroseconds(200);
    ambient_out = analogRead(rx_pin);

    // 2. قراءة الانعكاس والـ Emitter شغال
    digitalWrite(IR_EMITTERS_PIN, HIGH);
    delayMicroseconds(200);
    active_out = analogRead(rx_pin);

    // إطفاء الباعث لحفظ الطاقة وحمايته
    digitalWrite(IR_EMITTERS_PIN, LOW);

    // حساب القراءة الصافية (خصم الضوء المحيط)
    int clean = ambient_out - active_out;
    return (clean < 0) ? 0 : clean;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== IR SENSORS LIVE DIAGNOSTIC TEST ===");

    pinMode(IR_EMITTERS_PIN, OUTPUT);
    digitalWrite(IR_EMITTERS_PIN, LOW);

    pinMode(IR_LEFT_RX_PIN, INPUT);
    pinMode(IR_FRONT_RX_PIN, INPUT);
    pinMode(IR_RIGHT_RX_PIN, INPUT);
}

void loop() {
    int amb_L, act_L, amb_F, act_F, amb_R, act_R;

    int clean_L = read_ir_sensor(IR_LEFT_RX_PIN, amb_L, act_L);
    int clean_F = read_ir_sensor(IR_FRONT_RX_PIN, amb_F, act_F);
    int clean_R = read_ir_sensor(IR_RIGHT_RX_PIN, amb_R, act_R);

    // طباعة القراءات الصافية (CLEAN) لكل الحساسات والقراءات الخام (RAW) للحساس الأمامي
    Serial.printf("CLEAN -> L: %4d | F: %4d | R: %4d  ||  RAW (Active/Ambient) F: %4d / %4d\n",
                  clean_L, clean_F, clean_R, act_F, amb_F);

    delay(100);
}