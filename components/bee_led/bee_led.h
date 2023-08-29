#ifndef BEE_LED_H
#define BEE_LED_H

#include "driver/gpio.h"

#define RED_LED GPIO_NUM_12
#define GREEN_LED GPIO_NUM_13
#define BLUE_LED GPIO_NUM_14

void led_init();
void led_blink(uint8_t gpio_pin, bool on_color);


#endif