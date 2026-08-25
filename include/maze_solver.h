#ifndef MAZE_SOLVER_H
#define MAZE_SOLVER_H

#include <stdint.h>
#include <stdbool.h>



#define MAZE_SIZE 16
#define QUEUE_MAX_SIZE (MAZE_SIZE * MAZE_SIZE)

#define WALL_NORTH (1 << 0)
#define WALL_EAST (1 << 1)
#define WALL_SOUTH (1 << 2)
#define WALL_WEST (1 << 3)


typedef enum
{
    DIR_NORTH = 0,
    DIR_EAST = 1,
    DIR_SOUTH = 2,
    DIR_WEST = 3
} enDirection;

typedef enum {
    STATE_EXPLORE_TO_GOAL,
    STATE_RETURN_TO_START,
    STATE_SPEED_RUN
} enRobotState;

typedef struct
{
    uint8_t x;
    uint8_t y;
} stPosition;

typedef struct 
{
    stPosition data[QUEUE_MAX_SIZE];
    int front; 
    int rear;
    int count;
} Queue;

typedef struct
{
    uint8_t distance;
    uint8_t walls;
    bool visited;
} stCell;


void maze_init(void);
void flood_fill_recalculate(stPosition target_nodes[], uint8_t target_count);
enDirection maze_get_next_move(stPosition current_pos, enDirection current_dir);
void maze_update_wall(uint8_t x, uint8_t y, enDirection dir, bool wall_present);

void queue_init(Queue *q);
bool queue_push(Queue *q, stPosition pos);
stPosition queue_pop(Queue *q);
bool queue_is_empty(Queue *q);

uint8_t maze_get_distance(uint8_t x, uint8_t y);



#endif