#include "robot_hal.h"
#include <Arduino.h>
#include <Preferences.h>

#define PIN_LAUNCH_BUTTON 13
#define PIN_DIP_SWITCH_1  14
#define PIN_DIP_SWITCH_2  27

Preferences preferences;

void HAL_init(void) {
    Serial.begin(115200);
    
    pinMode(PIN_LAUNCH_BUTTON, INPUT_PULLUP);
    pinMode(PIN_DIP_SWITCH_1, INPUT_PULLUP);
    pinMode(PIN_DIP_SWITCH_2, INPUT_PULLUP);
    
    Serial.println("[HAL] Hardware Initialized Successfully.");
}

bool HAL_is_launch_button_pressed(void) {
    if (digitalRead(PIN_LAUNCH_BUTTON) == LOW) {
        delay(50); 
        if (digitalRead(PIN_LAUNCH_BUTTON) == LOW) {
            return true;
        }
    }
    return false;
}

uint8_t HAL_get_dip_switch_mode(void) {
    uint8_t bit1 = (digitalRead(PIN_DIP_SWITCH_1) == LOW) ? 1 : 0;
    uint8_t bit2 = (digitalRead(PIN_DIP_SWITCH_2) == LOW) ? 1 : 0;
    
    return (bit1 << 1) | bit2; 
}

void HAL_save_data_to_flash(const char* key, uint8_t* data, size_t size) {
    preferences.begin("mouse_data", false); 
    preferences.putBytes(key, data, size);  
    preferences.end();                      
    Serial.println("[HAL] Data saved to Flash Memory.");
}

bool HAL_load_data_from_flash(const char* key, uint8_t* data, size_t size) {
    preferences.begin("mouse_data", true);
    
    if (preferences.getBytesLength(key) == size) {
        preferences.getBytes(key, data, size);
        preferences.end();
        Serial.println("[HAL] Data loaded successfully from Flash.");
        return true;
    }
    
    preferences.end();
    Serial.println("[HAL] No valid data found in Flash.");
    return false;
}