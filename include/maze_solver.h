//To prevent Redefinition Error
#ifndef MAZE_SOLVER_H
#define MAZE_SOLVER_H


//To use uint8_t
#include <stdint.h>
#include <stdbool.h>


#define MAZE_SIZE 16
#define QUEUE_MAX_SIZE (MAZE_SIZE * MAZE_SIZE) //Queue to store total maze 16 * 16 = 256 cells


//To define walls as bits at one byte to reduce memory usage
#define WALL_NORTH (1 << 0)
#define WALL_EAST (1 << 1)
#define WALL_SOUTH (1 << 2)
#define WALL_WEST (1 << 3)

#ifdef __cplusplus
extern "C" {
#endif


//Determine directions
typedef enum
{
    DIR_NORTH = 0,
    DIR_EAST = 1,
    DIR_SOUTH = 2,
    DIR_WEST = 3
} enDirection;


//To determine robot state whether explore the maze, going to start or going through the shortest distance
typedef enum {
    STATE_EXPLORE_TO_GOAL,
    STATE_RETURN_TO_START,
    STATE_SPEED_RUN
} enRobotState;


//To determine positoin of every cell or maze at all
typedef struct
{
    uint8_t x;
    uint8_t y;
} stPosition;


//To make queue at store maze at it
typedef struct 
{
    stPosition data[QUEUE_MAX_SIZE];
    int front; 
    int rear;
    int count;
} Queue;


//To build the maze at robot memory
typedef struct
{
    uint8_t distance;
    uint8_t walls;
    bool visited;
} stCell;

extern stCell maze[MAZE_SIZE][MAZE_SIZE];


void queue_init(Queue *q); //To make new queue
bool queue_push(Queue *q, stPosition pos);// push cells at queue to check it
stPosition queue_pop(Queue *q);// pull cells to check it walls and update its number
bool queue_is_empty(Queue *q);// To check the queue is empty or not, if it's empty then the algorthim is done and stops


void maze_init(void);// To initilize the maze at robot memory
void maze_update_wall(uint8_t x, uint8_t y, enDirection dir, bool wall_present);// To update walls state when get data from senosrs


void flood_fill_recalculate(stPosition target_nodes[], uint8_t target_count);// To make numbers map and set the goal to zero
enDirection maze_get_next_move(stPosition current_pos, enDirection current_dir);// Find the suitable cell to get to it
uint8_t maze_get_distance(uint8_t x, uint8_t y);// get cells number, use it at simulator


//To make the robot ready for speed running, and get to the at through the shortest direction
void maze_save_to_flash(void);
bool maze_load_from_flash(void);


#ifdef __cplusplus
}
#endif

//To prevent Redefinition Error
#endif