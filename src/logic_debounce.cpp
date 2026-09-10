#include "logic_debounce.h"

bool debounce_should_accept(unsigned long now_ms, unsigned long last_accepted_ms, unsigned long debounce_window_ms) {
    // حساب الفرق الزمني بدقة لتجنب ارتداد الزر الميكانيكي
    return (now_ms - last_accepted_ms) >= debounce_window_ms; 
}