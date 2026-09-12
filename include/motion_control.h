#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include "maze_solver.h"

#ifdef __cplusplus
extern "C" {
#endif

void execute_forward_step();
void execute_turn(enDirection target_dir);

#ifdef __cplusplus
}
#endif

#endif 