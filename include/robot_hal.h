//This file is an abstraction file between alogrthim and hardware


//To prevent Redefinition Error
#ifndef ROBOT_HAL_H
#define ROBOT_HAL_H

#include <stdint.h>//To get uint8_t
#include <stdbool.h>//To get bool
#include <stddef.h>//To use size_t to prevent overflow


//To make c & c++ files work together without any problems
#ifdef __cplusplus
extern "C" {
#endif


void HAL_init(void);//it called one time, initilze all of Esp32 settings


bool HAL_is_launch_button_pressed(void);//Get state of launch botton
uint8_t HAL_get_dip_switch_mode(void);//Get switches state


//To upload and get the maze from memory
void HAL_save_data_to_flash(const char* key, uint8_t* data, size_t size);
bool HAL_load_data_from_flash(const char* key, uint8_t* data, size_t size);


#ifdef __cplusplus
}
#endif


//To prevent Redefinition Error
#endif