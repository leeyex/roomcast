#pragma once

#include "airsmart_devices.h"

#ifdef __cplusplus 
extern "C" {
#endif

int airsmart_gpio_dev_open(int *fd_airsmart, airsmart_gpio_function_e gpio_function);

int airsmart_gpio_read(int fd_airsmart, char *gpio_value);

int airsmart_get_gpio_neihe_status();

void* aux_handle(void *tag);

#ifdef __cplusplus 
}
#endif