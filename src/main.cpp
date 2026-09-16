// ===== FILE: src/main.cpp =====
#include <Arduino.h>
#include "robot_config.h"
#include "robot_hal.h"
#include "hw_motors.h"
#include "hw_sensors.h"
#include "motion_control.h"
#include "logic_wall_detection.h"

extern "C" {
    #include "maze_solver.h"
}

// Target center coordinates for 16x16 VictoRIS maze
stPosition center_targets[] = {{7, 7}, {7, 8}, {8, 7}, {8, 8}};

// Current robot position and orientation
stPosition current_pos = {0, 0};
enDirection current_dir = DIR_NORTH;

// Update grid coordinates based on absolute heading
void update_position_coordinates() {
    switch (current_dir) {
        case DIR_NORTH: current_pos.y++; break;
        case DIR_EAST:  current_pos.x++; break;
        case DIR_SOUTH: current_pos.y--; break;
        case DIR_WEST:  current_pos.x--; break;
    }
}

void setup() {
    // 1. Initialize Hardware Abstraction Layer & Peripherals
    HAL_init();
    motors_init();
    sensors_init(); // Initializes 3-Channel IR array & BMI160 IMU with auto-calibration
    maze_init();

    Serial.println("==========================================");
    Serial.println("   GREEN MOUSE - IEEE VICTORIS 5.0 RUN    ");
    Serial.println("==========================================");
    Serial.println("System Ready. Place robot in start cell...");
    
    delay(3000); // 3-second delay to position robot securely

    // ---------------------------------------------------
    // Maze Exploration Run (Flood Fill Algorithm)
    // ---------------------------------------------------
    bool reached_center = false;

    while (!reached_center) {
        // Step 1: Check if any center goal cell is reached
        for (int i = 0; i < 4; i++) {
            if (current_pos.x == center_targets[i].x && current_pos.y == center_targets[i].y) {
                reached_center = true;
                break;
            }
        }
        if (reached_center) break;

        // Step 2: Read 3 IR Sensors (Left, Front, Right) with Ambient Light Cancellation
        IRReadings ir = sensors_read_ir();

        // Step 3: Map relative sensor readings to absolute orientation walls
        WallDetectionResult walls = detect_walls(
            current_dir, 
            ir.front, ir.left, ir.right, 
            IR_WALL_THRESHOLD_FRONT, IR_WALL_THRESHOLD_SIDE
        );

        // Step 4: Update internal maze bitfield matrix
        maze_update_wall(current_pos.x, current_pos.y, walls.front_dir, walls.front_wall);
        maze_update_wall(current_pos.x, current_pos.y, walls.left_dir, walls.left_wall);
        maze_update_wall(current_pos.x, current_pos.y, walls.right_dir, walls.right_wall);

        // Step 5: Recalculate Flood Fill distance matrix (BFS propagation)
        flood_fill_recalculate(center_targets, 4);

        // Step 6: Query algorithm for optimal neighboring move
        enDirection next_move = maze_get_next_move(current_pos, current_dir);

        // Step 7: Execute zero-radius turn (90° / 180°) using BMI160 Gyro integration
        execute_turn(next_move);

        // Step 8: Move forward one cell (175mm) using Encoders & IR PD Wall Centering
        execute_forward_step();

        // Step 9: Update internal positional coordinates
        update_position_coordinates();
    }

    // Save explored maze data to ESP32 Flash Memory for Speed Run
    HAL_save_data_to_flash("maze_map", (uint8_t*)maze, sizeof(maze));
    Serial.println("Goal Reached! Maze mapped successfully & saved to Flash.");
}

void loop() {
    // Mission Complete: Halt all motor drives safely
    motors_stop();
    delay(100);
}