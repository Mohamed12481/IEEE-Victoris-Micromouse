#ifndef ROBOT_HAL_H
#define ROBOT_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void HAL_init(void);

bool HAL_is_launch_button_pressed(void);
uint8_t HAL_get_dip_switch_mode(void); 

void HAL_save_data_to_flash(const char* key, uint8_t* data, size_t size);
bool HAL_load_data_from_flash(const char* key, uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif