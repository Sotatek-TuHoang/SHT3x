/*****************************************************************************
 *
 * @file    main.c
 * @author  tuha
 * @date    17 August 2023
 * @brief   Lib deep sleep functionality with WiFi configuration
 *
 *****************************************************************************/

#ifndef BEE_DEEP_SLEEP_H
#define BEE_DEEP_SLEEP_H

#include "driver/gpio.h"

#define SECOND_10S 10
#define SECOND_20S 20
#define SECOND_30S 30

#define RESET_PIN 7


/**
 * @brief Task executed upon waking up from deep sleep.
 *
 * This function is responsible for handling tasks and operations after waking up from deep sleep.
 * It identifies the cause of wake-up (timer or GPIO) and performs corresponding actions.
 * If not in provisioning mode, it sends sensor data, keeps the connection alive, and prepares for the next sleep cycle.
 */
void deep_sleep_task(void *args);

/**
 * @brief Register RTC timer-based wake-up for deep sleep.
 *
 * This function enables the RTC timer wake-up source for the ESP32 deep sleep mode.
 * It configures the wake-up time interval in seconds and sets up the RTC timer for wake-up.
 * 
 * @param wakeup_time_sec time deep sleep
 */
void deep_sleep_register_rtc_timer_wakeup(uint8_t wakeup_time_sec);

/**
 * @brief Register external GPIO wake-up source for deep sleep.
 *
 * This function enables the specified GPIO pin to trigger wake-up from deep sleep mode.
 * It configures the GPIO pin to trigger a wake-up event when its state changes to low.
 * @param gpio_wakeup gpio set to wake up program from deep sleep
 */
void deep_sleep_register_gpio_wakeup(uint8_t gpio_wakeup);

#endif 
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/