/***************************************************************************
* @file         bee_sht3x.h
* @author       tuha
* @date         14 August 2023
* @brief        SHT3x sensor driver implementation.
*               This file contains the implementation of functions to
*               interact with the Sensirion SHT3x series temperature and
*               humidity sensor using the I2C communication protocol.
*               It provides functions for initializing the sensor, starting
*               measurements, reading raw data, computing temperature and
*               humidity values, and handling errors. The driver supports
*               both single-shot and periodic measurement modes.
*
****************************************************************************/

#ifndef SHT3x_H
#define SHT3x_H

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

//Define warning threshold
#define H_TEMP_THRESHOLD    30
#define H_HUMI_THRESHOLD    80
#define L_TEMP_THRESHOLD    25
#define L_HUMI_THRESHOLD    60
#define NO_WARNING 255

#define I2C_MASTER_TIMEOUT_MS   1000
#define I2C_MASTER_NUM          0
#define I2C_ACK_CHECK_DIS       0x00
#define I2C_ACK_CHECK_EN        0x01

#define SHT3X_SENSOR_ADDR       0x44
#define SHT3X_READ_ERROR        0xFFFF
#define SHT3X_HEX_CODE_SIZE     0x02

#define CRC8_POLYNOMIAL         0x31

#define MPS_0_5_REPEATABILITY_HIGH {0x20, 0x32}
#define MPS_0_5_REPEATABILITY_MEDIUM {0x20, 0x24}
#define MPS_0_5_REPEATABILITY_LOW {0x20, 0x2F}
#define MPS_1_REPEATABILITY_HIGH {0x21, 0x30}
#define MPS_1_REPEATABILITY_MEDIUM {0x21, 0x26}
#define MPS_1_REPEATABILITY_LOW {0x21, 0x2D}
#define MPS_2_REPEATABILITY_HIGH {0x22, 0x36}
#define MPS_2_REPEATABILITY_MEDIUM {0x22, 0x20}
#define MPS_2_REPEATABILITY_LOW {0x22, 0x2B}
#define MPS_4_REPEATABILITY_HIGH {0x23, 0x34}
#define MPS_4_REPEATABILITY_MEDIUM {0x23, 0x22}
#define MPS_4_REPEATABILITY_LOW {0x23, 0x29}
#define MPS_10_REPEATABILITY_HIGH {0x27, 0x37}
#define MPS_10_REPEATABILITY_MEDIUM {0x27, 0x21}
#define MPS_10_REPEATABILITY_LOW {0x27, 0x2A}

typedef struct sht3x_msb_lsb
{
    uint8_t msb;
    uint8_t lsb;
} sht3x_msb_lsb_t;

typedef struct sht3x_sensor_value
{
    sht3x_msb_lsb_t value;
    uint8_t crc;
} sht3x_sensor_value_t;

typedef struct sht3x_sensors_values
{
    float temperature;
    float humidity;
} sht3x_sensors_values_t;

typedef struct measurements
{
    sht3x_sensor_value_t temperature;
    sht3x_sensor_value_t humidity;
} measurements_t;

/**
 * @brief Start periodic measurement using the specified periodic command.
 *
 * @param periodic_command A pointer to the periodic command to start measurement.
 * @return ESP_OK if the command was sent successfully
 */
esp_err_t sht3x_start_periodic_measurement(uint8_t *periodic_command);

/**
 * @brief Start periodic measurement with the accelerated response time (ART) feature.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the operation was successful.
 */
esp_err_t sht3x_start_periodic_measurement_with_art();

/**
 * @brief Stop periodic measurement to change the sensor configuration or to save power.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the operation was successful.
 *         - An error code if the operation failed.
 *
 * @note The sensor will only respond to other commands after waiting 500 ms
 *       after issuing the stop_periodic_measurement command.
 */
esp_err_t sht3x_stop_periodic_measurement();

/**
 * @brief Read sensor output and calculate temperature and humidity values.
 *
 * This function reads the sensor output, calculates the temperature and humidity
 * values, and performs a CRC check on the received data to ensure its integrity.
 *
 * @param sensors_values A pointer to a structure where the temperature and humidity
 *                      values will be stored.
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the operation was successful.
 *         - ESP_ERR_INVALID_CRC if the CRC check fails, indicating corrupted data.
 *         - An error code if the operation failed for other reasons.
 */
esp_err_t sht3x_read_measurement(sht3x_sensors_values_t *sensors_values);

/**
 * @brief Perform a soft reset of the SHT3x sensor.
 *
 * The soft reset command triggers the sensor to reset its system controller and
 * reloads calibration data from memory. This function can be used to force the
 * system into a well-defined state without removing power.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the reset command was sent successfully.
 *         - An error code if the operation failed.
 */
esp_err_t sht3x_soft_reset();

/**
 * @brief Generate a reset of the SHT3x sensor using the "general call" mode.
 *
 * This function generates a reset of the sensor using the "general call" mode,
 * which is functionally identical to using the nReset pin. It follows the I2C-bus
 * specification for generating a reset.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the reset command was sent successfully.
 *         - An error code if the operation failed.
 */
esp_err_t sht3x_general_call_reset();

/**
 * @brief Enable the internal heater of the SHT3x sensor.
 *
 * This function enables the internal heater of the SHT3x sensor, which can be used
 * to raise the temperature inside the sensor for specific applications.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the heater was successfully enabled.
 *         - An error code if the operation failed.
 */
esp_err_t sht3x_enable_heater();

/**
 * @brief Disable the internal heater of the SHT3x sensor.
 *
 * This function disables the internal heater of the SHT3x sensor after enable.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the heater was successfully enabled.
 *         - An error code if the operation failed.
 */
esp_err_t sht3x_disable_heater();

/**
 * @brief Read the status register of the SHT3x sensor.
 *
 * The status register contains information on the operational status of the heater, the alert mode,
 * and on the execution status of the last command and the last write sequence.
 *
 * @param[out] sensors_value Pointer to a structure where the status register data will be stored.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the status register was successfully read.
 *         - SHT3X_READ_ERROR if the operation failed.
 */
esp_err_t sht3x_read_status_register(sht3x_sensor_value_t *sensors_value);

/**
 * @brief Clear all flags in the status register of the SHT3x sensor.
 *
 * This function clears all flags in the status register of the SHT3x sensor,
 * including the alert flags and any other status-related flags.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the status register was successfully cleared.
 *         - An error code if the operation failed.
 */
esp_err_t sht3x_clear_status_register();

/**
 * @brief Perform a single-shot measurement with the SHT3x sensor.
 *
 * This function performs a single-shot measurement with the SHT3x sensor using the specified repeatability mode.
 * The function first sends the appropriate command to start a single-shot measurement, then waits for the
 * measurement to complete, and finally reads the measurement data and calculates the temperature and humidity values.
 *
 * @param[out] sensors_values Pointer to a structure where the measurement data will be stored.
 *
 * @return An ESP error code indicating the success or failure of the operation.
 *         - ESP_OK if the single-shot measurement was successfully performed.
 *         - An error code if the operation failed.
 */
esp_err_t sht3x_read_singleshot(sht3x_sensors_values_t *sensors_values);

/**
 * @brief Check warnings based on temperature and humidity.
 *
 * This function checks predefined thresholds for temperature and humidity,
 * and returns a warning value based on the comparison of the measured temperature and humidity with the thresholds.
 *
 * @param[in] Temp Measured temperature.
 * @param[in] Humi Measured humidity.
 *
 * @return Warning value:
 *  - NO_WARNING if there are no warnings.
 *  - A different value represents a specific warning based on bits:
 *    - Bit 3: High-temperature warning.
 *    - Bit 2: Low-temperature warning.
 *    - Bit 1: High-humidity warning.
 *    - Bit 0: Low-humidity warning.
 */
uint8_t check_warning(float Temp, float Humi);

#endif /* __SHT3x_H__ */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/