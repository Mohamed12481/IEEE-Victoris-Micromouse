#include <Arduino.h>
#include "robot_config.h"
#include "robot_hal.h"
#include "hw_motors.h"
#include "hw_sensors.h"
#include "motion_control.h"

void setup() {
    HAL_init();
    motors_init();
    sensors_init();
    
    Serial.println("\n--- Momentum Calibration Menu ---");
    Serial.println("Send '1' -> Test IR Sensors");
    Serial.println("Send '2' -> Test BMI160 (IMU Drift)");
    Serial.println("Send '3' -> Test Motors (Wheels in air first!)");
    Serial.println("---------------------------------");
}

void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        if (cmd == '1') {
            Serial.println("Testing IR (Reading for 5 seconds)...");
            for(int i = 0; i < 50; i++) {
                IRReadings ir = sensors_read_ir();
                Serial.printf("IR -> Left: %d | Front: %d | Right: %d\n", ir.left, ir.front, ir.right);
                delay(100);
            }
        } 
        else if (cmd == '2') {
            Serial.println("Testing BMI160 Yaw (Keep Robot Still!)...");
            sensors_reset_yaw();
            unsigned long start = millis();
            while(millis() - start < 5000) {
                sensors_update_yaw(0.05f); // dt افتراضي للاختبار
                Serial.printf("Yaw Angle: %.2f degrees\n", sensors_get_yaw());
                delay(50);
            }
        }
        else if (cmd == '3') {
            Serial.println("Testing Motors (Forward 150 PWM for 1 sec)...");
            motors_set_pwm(150, 150);
            delay(1000);
            motors_stop();
            Serial.println("Motors Stopped.");
        }
    }
}