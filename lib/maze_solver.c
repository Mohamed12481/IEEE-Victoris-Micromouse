#include <stdlib.h>
#include "maze_solver.h"

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