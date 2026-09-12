#include "hw_safety.h"
#include <Arduino.h>
void safety_init() {
    // The schematic has no emergency-stop input. Do not configure a GPIO.
    Serial.println("[Safety] No hardware emergency-stop input; monitoring disabled.");
    // Intentionally no interrupt is attached.
}

bool safety_is_stopped() {
    return false;
}
