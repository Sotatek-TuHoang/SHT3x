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

void i2c_init(const i2c_cfg_init_t* config) 
{
    i2c_config_t i2c_conf;
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = config->sda_pin;
    i2c_conf.scl_io_num = config->scl_pin;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = config->frequency;
    
    ESP_ERROR_CHECK(i2c_param_config(config->bus, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(config->bus, I2C_MODE_MASTER, 0, 0, 0));
    i2c_filter_enable(config->bus, 1);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

