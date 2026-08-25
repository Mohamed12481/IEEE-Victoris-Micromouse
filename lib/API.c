#include "API.h"
#include <stdio.h>
#include <string.h>

bool API_wallFront(void) {
    printf("wallFront\n");
    fflush(stdout);
    char response[8];
    scanf("%s", response);
    return strcmp(response, "true") == 0;
}

bool API_wallRight(void) {
    printf("wallRight\n");
    fflush(stdout);
    char response[8];
    scanf("%s", response);
    return strcmp(response, "true") == 0;
}

bool API_wallLeft(void) {
    printf("wallLeft\n");
    fflush(stdout);
    char response[8];
    scanf("%s", response);
    return strcmp(response, "true") == 0;
}

void API_moveForward(void) {
    printf("moveForward\n");
    fflush(stdout);
    char response[8];
    scanf("%s", response);
}

void API_turnRight(void) {
    printf("turnRight\n");
    fflush(stdout);
    char response[8];
    scanf("%s", response);
}

void API_turnLeft(void) {
    printf("turnLeft\n");
    fflush(stdout);
    char response[8];
    scanf("%s", response);
}

void API_setText(int x, int y, const char* text) {
    printf("setText %d %d %s\n", x, y, text);
    fflush(stdout);
}

void API_setColor(int x, int y, char color) {
    printf("setColor %d %d %c\n", x, y, color);
    fflush(stdout);
}