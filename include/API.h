#ifndef API_H
#define API_H

#include <stdbool.h>

bool API_wallFront(void);
bool API_wallRight(void);
bool API_wallLeft(void);

void API_moveForward(void);
void API_turnRight(void);
void API_turnLeft(void);

void API_setText(int x, int y, const char* text);
void API_setColor(int x, int y, char color);

#endif