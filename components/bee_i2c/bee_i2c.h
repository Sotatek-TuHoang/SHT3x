/***************************************************************************
* @file 	bee_i2c.h
* @author 	tuha
* @date 	14 August 2023
* @brief    I2C communication functions implementation
***************************************************************************/

#ifndef BEE_I2C_H
#define BEE_I2C_H

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "driver/gpio.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define I2C_ACK_VAL  0x0
#define I2C_NACK_VAL 0x1
#define I2C_FREQ     100000
#define I2C_BUS      0
#define I2C_SCL_PIN  GPIO_NUM_3
#define I2C_SDA_PIN  GPIO_NUM_4

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/**
 * @brief Initialize the I2C bus with the specified configuration.
 *
 * This function initializes the I2C bus with the given parameters including the bus number, SCL and SDA pins,
 * clock frequency, and pull-up configuration. It configures the I2C bus in master mode and installs the I2C driver.
 *
 * @param bus I2C bus number to initialize.
 * @param scl GPIO number for the SCL (clock) line.
 * @param sda GPIO number for the SDA (data) line.
 * @param freq Clock frequency for the I2C bus.
 *
 * @return ESP_OK if the operation was successful
 */
void i2c_init(int bus, gpio_num_t scl, gpio_num_t sda, uint32_t freq);


#endif
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/