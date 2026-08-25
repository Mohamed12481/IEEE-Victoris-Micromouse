#include <stdio.h>
#include <stdbool.h>
#include "API.h"
#include "maze_solver.h"

static void clear_simulator_colors(void) {
    printf("clearAllColor\n");
    fflush(stdout);
}

static enDirection get_absolute_dir(enDirection current_dir, int relative_offset) {
    return (enDirection)((current_dir + relative_offset) % 4);
}

static void turn_to_direction(enDirection *current_dir, enDirection target_dir) {
    int diff = (target_dir - *current_dir + 4) % 4;
    if (diff == 1) {
        API_turnRight();
    } else if (diff == 3) {
        API_turnLeft();
    } else if (diff == 2) {
        API_turnRight();
        API_turnRight();
    }
    *current_dir = target_dir;
}

static void update_position(stPosition *pos, enDirection dir) {
    switch (dir) {
        case DIR_NORTH: pos->y++; break;
        case DIR_EAST:  pos->x++; break;
        case DIR_SOUTH: pos->y--; break;
        case DIR_WEST:  pos->x--; break;
    }
}

int main(void) {
    maze_init();

    stPosition current_pos = {0, 0};
    enDirection current_dir = DIR_NORTH;
    enRobotState current_state = STATE_EXPLORE_TO_GOAL;

    stPosition center_goals[4] = {{7, 7}, {7, 8}, {8, 7}, {8, 8}};
    stPosition start_goal[1]   = {{0, 0}};

    API_setText(0, 0, "START");
    API_setColor(0, 0, 'G'); 

    while (1) {
        bool wall_front = API_wallFront();
        bool wall_right = API_wallRight();
        bool wall_left  = API_wallLeft();

        maze_update_wall(current_pos.x, current_pos.y, get_absolute_dir(current_dir, 0), wall_front);
        maze_update_wall(current_pos.x, current_pos.y, get_absolute_dir(current_dir, 1), wall_right);
        maze_update_wall(current_pos.x, current_pos.y, get_absolute_dir(current_dir, 3), wall_left);

        char path_color = 'B'; 

        if (current_state == STATE_EXPLORE_TO_GOAL) {
            path_color = 'B'; 
            flood_fill_recalculate(center_goals, 4);

            if ((current_pos.x == 7 || current_pos.x == 8) && (current_pos.y == 7 || current_pos.y == 8)) {
                current_state = STATE_RETURN_TO_START;
                clear_simulator_colors();
                API_setText(current_pos.x, current_pos.y, "GOAL");
                API_setColor(current_pos.x, current_pos.y, 'Y');
                API_setColor(0, 0, 'G'); 
            }
        } 
        else if (current_state == STATE_RETURN_TO_START) {
            path_color = 'Y';
            flood_fill_recalculate(start_goal, 1);

            if (current_pos.x == 0 && current_pos.y == 0) {
                current_state = STATE_SPEED_RUN;
                clear_simulator_colors(); 
                API_setText(0, 0, "READY");
                API_setColor(0, 0, 'R');
                
                API_setColor(7, 7, 'G');
                API_setColor(7, 8, 'G');
                API_setColor(8, 7, 'G');
                API_setColor(8, 8, 'G');
            }
        } 
        else if (current_state == STATE_SPEED_RUN) {
            path_color = 'G'; 
            flood_fill_recalculate(center_goals, 4);

            if ((current_pos.x == 7 || current_pos.x == 8) && (current_pos.y == 7 || current_pos.y == 8)) {
                API_setText(current_pos.x, current_pos.y, "WIN!");
                break; 
            }
        }

        API_setColor(current_pos.x, current_pos.y, path_color);

        for (int x = 0; x < MAZE_SIZE; x++) {
            for (int y = 0; y < MAZE_SIZE; y++) {
                char text[4];
                sprintf(text, "%d", maze_get_distance(x, y));
                API_setText(x, y, text);
            }
        }

        enDirection next_dir = maze_get_next_move(current_pos, current_dir);
        turn_to_direction(&current_dir, next_dir);
        API_moveForward();
        update_position(&current_pos, current_dir);
    }

    return 0;
}