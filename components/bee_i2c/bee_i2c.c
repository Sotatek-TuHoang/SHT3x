/***************************************************************************
* @file 	bee_i2c.c
* @author 	tuha
* @date 	14 August 2023
* @brief    I2C communication functions implementation
***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <sys/time.h>
#include <string.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"

#include "bee_i2c.h"

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void i2c_init (int bus, gpio_num_t scl, gpio_num_t sda, uint32_t freq)
{
    static i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = freq;
    ESP_ERROR_CHECK(i2c_param_config(bus, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(bus, I2C_MODE_MASTER, 0, 0, 0));
    i2c_filter_enable(bus, 1);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

