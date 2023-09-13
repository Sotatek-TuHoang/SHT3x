/*****************************************************************************
 *
 * @file    main.c
 * @author  tuha
 * @date    17 August 2023
 * @brief   Lib control led
 *
 *****************************************************************************/

#ifndef BEE_LEDC_H
#define BEE_LEDC_H

#include "driver/gpio.h"
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE

#define GREEN_LEDC              GPIO_NUM_4
#define BLUE_LEDC               GPIO_NUM_5

#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits

#define LEDC_DUTY_0             (0)
#define LEDC_DUTY_10            (819)
#define LEDC_DUTY_50            (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

void ledc_init(uint8_t gpio_num, uint8_t channel);

void ledc_on(uint8_t channel, uint16_t duty);

void ledc_off(uint8_t channel);

#endif 
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/