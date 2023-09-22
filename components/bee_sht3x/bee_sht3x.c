/***************************************************************************
* @file         bee_sht3x.c
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "esp_log.h"
#include "math.h"
#include <sys/time.h>
#include "math.h"

#include "bee_i2c.h"
#include "bee_sht3x.h"

static const char *SHT3X_TAG = "sht3x";

// Single Shot Data Acquisition
uint8_t clock_stretching_enabled_repeatability_high[]     = {0x2C, 0x06};
uint8_t clock_stretching_enabled_repeatability_medium[]   = {0x2C, 0x0D};
uint8_t clock_stretching_enabled_repeatability_low[]      = {0x2C, 0x10};
uint8_t clock_stretching_disabled_repeatability_high[]    = {0x24, 0x00};
uint8_t clock_stretching_disabled_repeatability_medium[]  = {0x24, 0x0B};
uint8_t clock_stretching_disabled_repeatability_low[]     = {0x24, 0x16};

// Commands
uint8_t read_measurement[]              = {0xE0, 0x00};
uint8_t periodic_measurement_with_art[] = {0x2B, 0x32};
uint8_t stop_periodic_measurement[]     = {0x30, 0x93};
uint8_t soft_reset[]                    = {0x30, 0xA2};
uint8_t general_call_reset[]            = {0x00, 0x06};
uint8_t heater_enable[]                 = {0x30, 0x6D};
uint8_t heater_disable[]                = {0x30, 0x66};
uint8_t read_status_register[]          = {0xF3, 0x2D};
uint8_t clear_status_register[]         = {0x30, 0x41};

/**
 * @brief Calculate the 8-bit CRC checksum for data.
 *
 * This function calculates the 8-bit CRC checksum for a given data buffer.
 * The CRC covers the contents of the data bytes in the buffer.
 *
 * @param[in] data Pointer to the data buffer.
 * @param[in] data_len Length of the data buffer.
 *
 * @return The calculated CRC checksum.
 */
static uint8_t calculate_crc(const uint8_t* data, uint8_t data_len) {
    uint16_t current_byte;
    uint8_t crc = 0xFF;
    uint8_t crc_bit;

    for(current_byte = 0; current_byte < data_len; ++current_byte)
    {
        crc ^= (data[current_byte]);
        for(crc_bit = 8; crc_bit > 0; --crc_bit)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

/*
* For the send command sequences, after writing the address and/or data to the sensor
* and sending the ACK bit, the sensor needs the execution time to respond to the I2C read header with an ACK bit.
* Hence, it is required to wait the command execution time before issuing the read header.
* Commands must not be sent while a previous command is being processed.
*/
static esp_err_t sht3x_send_command(uint8_t *command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_start(cmd));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmd, (SHT3X_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN));

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write(cmd, command, sizeof(command), I2C_ACK_CHECK_EN));

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_stop(cmd));
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);
    return err;
}

/*
* Data sent to and received from the sensor consists of a sequence of 16-bit commands and/or 16-bit words
* (each to be interpreted as unsigned integer, most significant byte transmitted first). Each data word is
* immediately succeeded by an 8-bit CRC. In write direction it is mandatory to transmit the checksum.
* In read direction it is up to the master to decide if it wants to process the checksum.
*/
static esp_err_t sht3x_read(uint8_t *hex_code, uint8_t *measurements, uint8_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_start(cmd));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmd, (SHT3X_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write(cmd, hex_code, SHT3X_HEX_CODE_SIZE, I2C_ACK_CHECK_EN));

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_start(cmd));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmd, (SHT3X_SENSOR_ADDR << 1) | I2C_MASTER_READ, I2C_ACK_CHECK_EN));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_read(cmd, measurements, size, I2C_MASTER_LAST_NACK));

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_stop(cmd));
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);
    return err;
}

esp_err_t sht3x_start_periodic_measurement(uint8_t *periodic_command)
{
    return sht3x_send_command(periodic_command);
}

esp_err_t sht3x_start_periodic_measurement_with_art()
{
    return sht3x_send_command(periodic_measurement_with_art);
}

esp_err_t sht3x_stop_periodic_measurement()
{
    return sht3x_send_command(stop_periodic_measurement);
}

esp_err_t sht3x_read_measurement(sht3x_sensors_values_t *sensors_values)
{
    measurements_t measurements =
    {
        .temperature = {{0x00, 0x00}, 0x00},
        .humidity = {{0x00, 0x00}, 0x00}
    };

    esp_err_t err = sht3x_read(read_measurement, (uint8_t *) &measurements, sizeof(measurements));

    if (calculate_crc((uint8_t *)&measurements.temperature, sizeof(measurements.temperature.value)) != measurements.temperature.crc ||
        calculate_crc((uint8_t *)&measurements.humidity, sizeof(measurements.humidity.value)) != measurements.humidity.crc)
    {
        return ESP_ERR_INVALID_CRC;
    }

    sensors_values->temperature = (175.0 * (((measurements.temperature.value.msb << 8) + measurements.temperature.value.lsb) / 65535.0)) - 45.0;
    sensors_values->humidity = 100.0 * ((measurements.humidity.value.msb << 8) + measurements.humidity.value.lsb) / 65535.0;

    return err;
}

esp_err_t sht3x_soft_reset()
{
    return sht3x_send_command(soft_reset);
}

esp_err_t sht3x_general_call_reset()
{
    return sht3x_send_command(general_call_reset);
}

esp_err_t sht3x_enable_heater()
{
    return sht3x_send_command(heater_enable);
}

esp_err_t sht3x_disable_heater()
{
    return sht3x_send_command(heater_disable);
}

esp_err_t sht3x_read_status_register(sht3x_sensor_value_t *sensors_value)
{
    sht3x_sensor_value_t status_register =
    {
        .value = {0x00, 0x00},
        .crc = 0x00
    };

    esp_err_t err = sht3x_read(read_status_register, (uint8_t *) &status_register, sizeof(status_register));

    if(err != ESP_OK)
    {
        ESP_LOGE(SHT3X_TAG, "read_status_register failed with status code: %s", esp_err_to_name(err));
        return SHT3X_READ_ERROR;
    }
    return err;
}

esp_err_t sht3x_clear_status_register() {
    return sht3x_send_command(clear_status_register);
}

esp_err_t sht3x_read_singleshot(sht3x_sensors_values_t *sensors_values)
{
    esp_err_t err = sht3x_send_command(clock_stretching_enabled_repeatability_high);
    if (err != ESP_OK)
    {
        ESP_LOGE(SHT3X_TAG, "sht3x_send_command failed with status code: %s", esp_err_to_name(err));
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(40));
    return sht3x_read_measurement(sensors_values);
}

RTC_DATA_ATTR int u8warning_values;
uint8_t check_warning(float Temp, float Humi)
{
    bool bH_Temp_threshold = Temp > H_TEMP_THRESHOLD;
    bool bL_Temp_threshold = Temp < L_TEMP_THRESHOLD;
    bool bH_Humi_threshold = Humi > H_HUMI_THRESHOLD;
    bool bL_Humi_threshold = Humi < L_HUMI_THRESHOLD;

    uint8_t u8tmp_warning_values = (bH_Temp_threshold << 3) | (bL_Temp_threshold << 2) | (bH_Humi_threshold << 1) | bL_Humi_threshold;

    if (u8tmp_warning_values != u8warning_values)
    {
        u8warning_values = u8tmp_warning_values;
        return u8warning_values;
    }
    else
    {
        return NO_WARNING;
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/