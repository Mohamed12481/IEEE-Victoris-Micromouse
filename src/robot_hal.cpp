#include "robot_hal.h"
#include <Arduino.h>//esp32 frame work, and get some functions
#include <Preferences.h>//esp32 library to control memory


Preferences preferences;//make an object to control memory


void HAL_init(void) {
    Serial.begin(115200);//open UART between robot and computer for debugging
    
    Serial.println("[HAL] Hardware Initialized Successfully.");
}


bool HAL_is_launch_button_pressed(void) {
    // The schematic has no launch-button circuit. Do not reuse motor GPIOs.
    return true;
}


uint8_t HAL_get_dip_switch_mode(void) {
    // The schematic has no DIP-switch circuit. Default to search mode.
    return 0;
}


//open space at memory to write the maze at it and upload it to flash
void HAL_save_data_to_flash(const char* key, uint8_t* data, size_t size) {
    preferences.begin("mouse_data", false); 
    preferences.putBytes(key, data, size);//move from RAM to Flash
    preferences.end();                      
    Serial.println("[HAL] Data saved to Flash Memory.");
}


//read maze from flash
bool HAL_load_data_from_flash(const char* key, uint8_t* data, size_t size) {
    preferences.begin("mouse_data", true);
    
    if (preferences.getBytesLength(key) == size) {
        preferences.getBytes(key, data, size);//get from Flash to RAM
        preferences.end();
        Serial.println("[HAL] Data loaded successfully from Flash.");
        return true;
    }
    
    preferences.end();
    Serial.println("[HAL] No valid data found in Flash.");
    return false;
}
