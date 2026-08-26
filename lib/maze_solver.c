#include <stdlib.h>
#include "maze_solver.h"
#include "robot_hal.h"

void maze_save_to_flash(void) {
    HAL_save_data_to_flash("maze_data", (uint8_t*)maze, sizeof(maze));
}

bool maze_load_from_flash(void) {
    return HAL_load_data_from_flash("maze_data", (uint8_t*)maze, sizeof(maze));
}

static stCell maze[MAZE_SIZE][MAZE_SIZE];
static Queue update_queue;

void queue_init(Queue *q)
{
    q->front = 0;
    q->rear = -1;
    q->count = 0;
} 

bool queue_is_empty(Queue *q) {
    return q->count == 0;
}

bool queue_push(Queue *q, stPosition pos) {
    if (q->count >= QUEUE_MAX_SIZE) return false;
    q->rear = (q->rear + 1) % QUEUE_MAX_SIZE;
    q->data[q->rear] = pos;
    q->count++;
    return true;
}

stPosition queue_pop(Queue *q) {
    stPosition empty = {0, 0};
    if (queue_is_empty(q)) return empty;
    stPosition pos = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_MAX_SIZE;
    q->count--;
    return pos;
}

void maze_init(void) {
    queue_init(&update_queue);

    for (uint8_t x = 0; x < MAZE_SIZE; x++) {
        for (uint8_t y = 0; y < MAZE_SIZE; y++) {
            maze[x][y].walls = 0;
            maze[x][y].visited = false;

            uint8_t dx = (x < 8) ? (7 - x) : (x - 8);
            uint8_t dy = (y < 8) ? (7 - y) : (y - 8);
            maze[x][y].distance = dx + dy;

            if (y == MAZE_SIZE - 1) maze[x][y].walls |= WALL_NORTH;
            if (x == MAZE_SIZE - 1) maze[x][y].walls |= WALL_EAST;
            if (y == 0)             maze[x][y].walls |= WALL_SOUTH;
            if (x == 0)             maze[x][y].walls |= WALL_WEST;
        }
    }
}

void maze_update_wall(uint8_t x, uint8_t y, enDirection dir, bool wall_present) {
    if (!wall_present) return;

    switch (dir) {
        case DIR_NORTH:
            maze[x][y].walls |= WALL_NORTH;
            if (y < MAZE_SIZE - 1) maze[x][y + 1].walls |= WALL_SOUTH;
            break;
        case DIR_EAST:
            maze[x][y].walls |= WALL_EAST;
            if (x < MAZE_SIZE - 1) maze[x + 1][y].walls |= WALL_WEST;
            break;
        case DIR_SOUTH:
            maze[x][y].walls |= WALL_SOUTH;
            if (y > 0) maze[x][y - 1].walls |= WALL_NORTH;
            break;
        case DIR_WEST:
            maze[x][y].walls |= WALL_WEST;
            if (x > 0) maze[x - 1][y].walls |= WALL_EAST;
            break;
    }
}

enDirection maze_get_next_move(stPosition current_pos, enDirection current_dir) {
    uint8_t min_dist = 255;
    enDirection best_dir = current_dir;

    uint8_t x = current_pos.x;
    uint8_t y = current_pos.y;

    if (!(maze[x][y].walls & WALL_NORTH) && y < MAZE_SIZE - 1) {
        if (maze[x][y + 1].distance < min_dist) {
            min_dist = maze[x][y + 1].distance;
            best_dir = DIR_NORTH;
        }
    }
    if (!(maze[x][y].walls & WALL_EAST) && x < MAZE_SIZE - 1) {
        if (maze[x + 1][y].distance < min_dist) {
            min_dist = maze[x + 1][y].distance;
            best_dir = DIR_EAST;
        }
    }
    if (!(maze[x][y].walls & WALL_SOUTH) && y > 0) {
        if (maze[x][y - 1].distance < min_dist) {
            min_dist = maze[x][y - 1].distance;
            best_dir = DIR_SOUTH;
        }
    }
    if (!(maze[x][y].walls & WALL_WEST) && x > 0) {
        if (maze[x - 1][y].distance < min_dist) {
            min_dist = maze[x - 1][y].distance;
            best_dir = DIR_WEST;
        }
    }

    return best_dir;
}

void flood_fill_recalculate(stPosition target_nodes[], uint8_t target_count) {
    for (uint8_t x = 0; x < MAZE_SIZE; x++) {
        for (uint8_t y = 0; y < MAZE_SIZE; y++) {
            maze[x][y].distance = 255; 
        }
    }

    queue_init(&update_queue);
    
    for (uint8_t i = 0; i < target_count; i++) {
        maze[target_nodes[i].x][target_nodes[i].y].distance = 0; 
        queue_push(&update_queue, target_nodes[i]);             
    }

    while (!queue_is_empty(&update_queue)) {
        stPosition curr = queue_pop(&update_queue);
        uint8_t cx = curr.x;
        uint8_t cy = curr.y;
        
        uint8_t next_dist = maze[cx][cy].distance + 1;

        if (!(maze[cx][cy].walls & WALL_NORTH) && cy < MAZE_SIZE - 1) {
            if (maze[cx][cy + 1].distance == 255) { 
                maze[cx][cy + 1].distance = next_dist;
                stPosition next_pos = {cx, cy + 1};
                queue_push(&update_queue, next_pos);
            }
        }
        if (!(maze[cx][cy].walls & WALL_EAST) && cx < MAZE_SIZE - 1) {
            if (maze[cx + 1][cy].distance == 255) {
                maze[cx + 1][cy].distance = next_dist;
                stPosition next_pos = {cx + 1, cy};
                queue_push(&update_queue, next_pos);
            }
        }
        if (!(maze[cx][cy].walls & WALL_SOUTH) && cy > 0) {
            if (maze[cx][cy - 1].distance == 255) {
                maze[cx][cy - 1].distance = next_dist;
                stPosition next_pos = {cx, cy - 1};
                queue_push(&update_queue, next_pos);
            }
        }
        if (!(maze[cx][cy].walls & WALL_WEST) && cx > 0) {
            if (maze[cx - 1][cy].distance == 255) {
                maze[cx - 1][cy].distance = next_dist;
                stPosition next_pos = {cx - 1, cy};
                queue_push(&update_queue, next_pos);
            }
        }
    }
}

uint8_t maze_get_distance(uint8_t x, uint8_t y) {
    return maze[x][y].distance;
}