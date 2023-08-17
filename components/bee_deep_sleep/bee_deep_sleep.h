#ifndef BEE_DEEP_SLEEP_H
#define BEE_DEEP_SLEEP_H

#include "driver/gpio.h"

#define GPIO_WAKEUP GPIO_NUM_2

void deep_sleep_task(void *args);

void deep_sleep_register_rtc_timer_wakeup(void);

void deep_sleep_register_ext1_wakeup(void);

#endif 