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
/***        Variable Definiti                                             ***/
/****************************************************************************/

/**Define variable*/
#define H_TEMP_THRESHOLD 30
#define H_HUMI_THRESHOLD 80
#define L_TEMP_THRESHOLD 25
#define L_HUMI_THRESHOLD 60
#define NO_WARNNG 255

#define I2C_MASTER_TIMEOUT_MS       (1000)
#define I2C_MASTER_NUM              (0)
#define I2C_ACK_CHECK_DIS           (0x00)
#define I2C_ACK_CHECK_EN            (0x01)

#define SHT3X_SENSOR_ADDR           (0x44)
#define SHT3X_READ_ERROR            (0xFFFF)
#define SHT3X_HEX_CODE_SIZE         (0x02)

#define CRC8_POLYNOMIAL             (0x31)
#define CRC8_INIT                   (0xFF)

typedef struct sht3x_msb_lsb {
    uint8_t msb;
    uint8_t lsb;
} sht3x_msb_lsb_t;

typedef struct sht3x_sensor_value {
    sht3x_msb_lsb_t value;
    uint8_t crc;
} sht3x_sensor_value_t;

typedef struct sht3x_sensors_values {
    float temperature;
    float humidity;
} sht3x_sensors_values_t;

uint8_t sht3x_generate_crc(const uint8_t* data, uint16_t count);

esp_err_t sht3x_send_command(uint8_t *command);

esp_err_t sht3x_read(uint8_t *hex_code, uint8_t *measurements, uint8_t size);

esp_err_t sht3x_write(uint8_t *hex_code, uint8_t *measurements, uint8_t size);

esp_err_t sht3x_send_command_and_fetch_result(uint8_t *command, uint8_t *measurements, uint8_t size);

esp_err_t sht3x_start_periodic_measurement();

esp_err_t sht3x_start_periodic_measurement_with_art();

esp_err_t sht3x_read_measurement(sht3x_sensors_values_t *sensors_values);

esp_err_t sht3x_stop_periodic_measurement();

esp_err_t sht3x_soft_reset();

esp_err_t sht3x_general_call_reset();

esp_err_t sht3x_enable_heater();

esp_err_t sht3x_disable_heater();

esp_err_t sht3x_read_status_register(sht3x_sensor_value_t *sensors_value);

esp_err_t sht3x_clear_status_register();

esp_err_t sht3x_read_singleshot(sht3x_sensors_values_t *sensors_values);

uint8_t check_warning(float fTemp, float fHumi);

#endif /* __SHT3x_H__ */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/