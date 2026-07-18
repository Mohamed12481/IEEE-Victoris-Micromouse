#include <stdio.h>
#include <stdint.h>  
#include <stdbool.h>

#define MAZE_SIZE 16

#define WALL_NORTH (1 << 0) //  00000001 
#define WALL_EAST  (1 << 1) //  00000010 
#define WALL_SOUTH (1 << 2) //  00000100 
#define WALL_WEST  (1 << 3) //  00001000 

uint8_t maze_walls[MAZE_SIZE][MAZE_SIZE];