#include <stdio.h>
#include <stdint.h>  
#include <stdalign.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAZE_SIZE 16

#define WALL_NORTH (1 << 0) //  00000001 
#define WALL_EAST  (1 << 1) //  00000010 
#define WALL_SOUTH (1 << 2) //  00000100 
#define WALL_WEST  (1 << 3) //  00001000 

uint8_t maze_walls[MAZE_SIZE][MAZE_SIZE];
uint8_t maze_values[MAZE_SIZE][MAZE_SIZE];

uint8_t get_distance_to_center(int x, int y)
{
    int target_x;
    int target_y;

    if(x < 8) target_x = 7;
    else target_x = 8;

    if (y < 8) target_y = 7;
    else target_y = 8;

    return abs(x - target_x) + abs(y - target_y);
}

void initialize_maze_values() {
    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int y = 0; y < MAZE_SIZE; y++) {
            maze_values[x][y] = get_distance_to_center(x, y);
        }
    }
}

int main()
{
    initialize_maze_values();

    printf("--- Initial Flood Fill Maze Values (16x16) ---\n\n");

    for (int y = (MAZE_SIZE - 1); y >= 0; y--)
    {
        for (int x = 0; x < MAZE_SIZE; x++)
        {
            printf("%3d ", maze_values[x][y]);
        }
        printf("\n");
    }

    printf("\n----------------------------------------------\n");
    return 0;
}