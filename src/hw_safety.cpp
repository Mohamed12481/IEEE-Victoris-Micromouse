#include "hw_safety.h"
#include <Arduino.h>
#include "robot_config.h"
#include "logic_debounce.h"

#define DEBOUNCE_WINDOW_MS 50

// volatile لأنها بتتغير جوه interrupt (نفس مبدأ عدادات الإنكودر)
static volatile bool g_emergency_triggered = false;
static volatile unsigned long g_last_accepted_ms = 0;

void IRAM_ATTR isr_emergency_stop() {
    unsigned long now = millis();
    if (debounce_should_accept(now, g_last_accepted_ms, DEBOUNCE_WINDOW_MS)) {
        g_emergency_triggered = true;
        g_last_accepted_ms = now;
    }
}

void safety_init() {
    // بين input only، مفيهوش pull-up داخلي - لازم مقاومة خارجية (شرح في robot_config.h)
    pinMode(EMERGENCY_STOP_PIN, INPUT);
    // FALLING: بيتفعل لحظة الضغط على الزرار (من HIGH لـ LOW بسبب الـ pull-up الخارجي)
    attachInterrupt(digitalPinToInterrupt(EMERGENCY_STOP_PIN), isr_emergency_stop, FALLING);
}

bool safety_is_stopped() {
    return g_emergency_triggered;
}