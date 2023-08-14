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
#define I2C_SCL_PIN  22
#define I2C_SDA_PIN  21

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/**
 * @brief Get the system time in microseconds.
 *
 * This function retrieves the current system time in microseconds since an arbitrary reference point.
 * It uses the gettimeofday() function to obtain the time.
 *
 * @return The current system time in microseconds.
 */
uint32_t sdk_system_get_time();

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
 * @return None
 */
void i2c_init(int bus, gpio_num_t scl, gpio_num_t sda, uint32_t freq);

/**
 * @brief Write data to an I2C slave device.
 *
 * This function writes data to an I2C slave device specified by the given bus number, address, register,
 * data buffer, and length. It creates an I2C command handle, sends start signal, writes the slave address
 * and register (if provided), writes the data, sends stop signal, and executes the I2C command.
 *
 * @param bus I2C bus number to communicate on.
 * @param addr I2C slave device address.
 * @param reg Pointer to the register address data (can be NULL if not needed).
 * @param data Pointer to the data buffer to be written.
 * @param len Length of the data to be written.
 *
 * @return ESP_OK if the operation was successful, otherwise an error code.
 */
int i2c_slave_write(uint8_t bus, uint8_t addr, const uint8_t *reg, 
                     uint8_t *data, uint32_t len);

/**
 * @brief Read data from an I2C slave device.
 *
 * This function reads data from an I2C slave device specified by the given bus number, address, register,
 * data buffer, and length. It creates an I2C command handle, sends start signal, writes the slave address
 * and register (if provided), reads the data, sends stop signal, and executes the I2C command.
 *
 * @param bus I2C bus number to communicate on.
 * @param addr I2C slave device address.
 * @param reg Pointer to the register address data (can be NULL if not needed).
 * @param data Pointer to the data buffer to store read data.
 * @param len Length of the data to be read.
 *
 * @return ESP_OK if the operation was successful, otherwise an error code.
 */
int i2c_slave_read(uint8_t bus, uint8_t addr, const uint8_t *reg, 
                    uint8_t *data, uint32_t len);

#endif
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/