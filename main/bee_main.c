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
    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    deep_sleep_register_rtc_timer_wakeup(SECOND_10S);
    deep_sleep_register_ext1_wakeup(GPIO_NUM_0);
    button_init(GPIO_NUM_0);
    xTaskCreate(deep_sleep_task, "deep_sleep_task", 4096, NULL, 10, NULL);
}
