/*****************************************************************************
 *
 * @file     bee_main.c
 * @author   tuha
 * @date     21 August 2023
 * @brief    Control program for a device measuring temperature and humidity,
 *           connecting to Wi-Fi, sending data via MQTT, and managing deep sleep mode.
 *
 *****************************************************************************/

#include "bee_sht3x.h"
#include "bee_i2c.h"
#include "bee_wifi.h"
#include "bee_mqtt.h"
#include "bee_nvs.h"
#include "bee_deep_sleep.h"
#include "bee_button.h"
#include "driver/gpio.h"

void app_main(void)
{
    i2c_cfg_init_t i2c_config = {
        .bus = I2C_NUM_0,
        .scl_pin = GPIO_NUM_7,
        .sda_pin = GPIO_NUM_8,
        .frequency = 100000 
    };
    i2c_init(&i2c_config);

    deep_sleep_register_rtc_timer_wakeup(SECOND_30S);

    deep_sleep_register_gpio_wakeup(GPIO_NUM_2);

    button_init(GPIO_NUM_2);

    xTaskCreate(deep_sleep_task, "deep_sleep_task", 4096, NULL, 31, NULL);
}
