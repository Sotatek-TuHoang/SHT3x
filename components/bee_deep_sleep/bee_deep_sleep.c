/*****************************************************************************
 *
 * @file    main.c
 * @author  tuha
 * @date    17 August 2023
 * @brief   Lib deep sleep functionality with WiFi configuration
 *
 *****************************************************************************/

/****************************************************************************/
/***        Include                                                       ***/
/****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "driver/i2c.h"
#include <esp_system.h>
#include <esp_system.h>

#include "bee_nvs.h"
#include "bee_deep_sleep.h"
#include "bee_mqtt.h"
#include "bee_sht3x.h"
#include "bee_i2c.h"
#include "bee_wifi.h"

/****************************************************************************/
/***        Global Variables                                              ***/
/****************************************************************************/

bool bInit;

/****************************************************************************/
/***        Static Variables                                              ***/
/****************************************************************************/
// storage variables to rtc memory, so variables dont reset after wake up from deep sleep
static RTC_DATA_ATTR struct timeval sleep_enter_time; 
static RTC_DATA_ATTR uint8_t u8cnt_sleep = 0;

static float fTemp;
static float fHumi;

// Define tags for log messages
static const char *TAG_SHT3x = "SHT3x";
static const char *TAG_PM = "POWER MODE";

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

static void init_resource_pub_mqtt()
{
    if (!bInit)
    {
        nvs_flash_func_init();
        wifi_func_init();
        mqtt_func_init();
        bInit = true;
    }
}

static void check_and_pub_warning()
{
    uint8_t u8Warning_value = check_warning(fTemp, fHumi);
    if (u8Warning_value != NO_WARNNG)
    {
        init_resource_pub_mqtt();
        pub_warning(u8Warning_value, fTemp, fHumi);
        vTaskDelay (30 / portTICK_PERIOD_MS);
        mqtt_disconnect();
        esp_wifi_disconnect();
    }
}

static bool read_data(void)
{
    // Initialize and measure using the SHT3x sensor
    sht3x_sensor_t* sensor;
    if ((sensor = sht3x_init_sensor (I2C_BUS, SHT3x_ADDR_1)))
    {
        if (sht3x_measure (sensor, &fTemp, &fHumi))
        {
            ESP_LOGI(TAG_SHT3x, "Temperature: %.2f °C, Humidity: %.2f %%", fTemp, fHumi);
            return true;
        }
    }
    gpio_reset_pin(RESET_PIN);
    gpio_set_pull_mode(RESET_PIN, GPIO_PULLUP_ONLY);
    gpio_set_direction(RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RESET_PIN, 0);
    return false;
}

static void check_cause_wake_up(void)
{
    // Get current time and calculate sleep time
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause()) // Handle different wake-up causes
    {
        case ESP_SLEEP_WAKEUP_TIMER:
        {
            ESP_LOGI(TAG_PM, "Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);

            if (u8cnt_sleep == 4)
            {
                u8cnt_sleep = 0;
                
                if (read_data())
                {
                    init_resource_pub_mqtt();
                    pub_data(fTemp, fHumi);
                    vTaskDelay (20 / portTICK_PERIOD_MS);
                    mqtt_disconnect();
                    esp_wifi_disconnect();                
                }
            }
            else
            {   
                u8cnt_sleep++;
                if (read_data())
                {
                    check_and_pub_warning();
                } 
            }
            break;
        }

        case ESP_SLEEP_WAKEUP_GPIO:
        {
            ESP_LOGI(TAG_PM, "Wakeup from GPIO\n");
            vTaskDelay (12000 / portTICK_PERIOD_MS); //Wait 12 sec for button press
            extern bool bButton_task;
            while (bButton_task) //Delay sleep to complete config wifi or OTA
            {
                vTaskDelay (500 / portTICK_PERIOD_MS);
            }
            break;
        }

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            ESP_LOGI(TAG_PM, "Not a deep sleep reset\n");
    }
}

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void deep_sleep_register_rtc_timer_wakeup(uint8_t wakeup_time_sec)
{
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
}

void deep_sleep_register_gpio_wakeup(uint8_t gpio_wakeup)
{
    const gpio_config_t config = {
        .pin_bit_mask = BIT(gpio_wakeup),
        .mode = GPIO_MODE_INPUT,
    };

    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(gpio_wakeup), 0));

    printf("Enabling GPIO wakeup on pins GPIO%d\n", gpio_wakeup);
}

/****************************************************************************/
/***        Task                                                          ***/
/****************************************************************************/

void deep_sleep_task(void *args)
{    
    ESP_LOGI(TAG_PM, "Entering normal mode\n");
    check_cause_wake_up();

    gettimeofday(&sleep_enter_time, NULL); // Get deep sleep enter time
    ESP_LOGI(TAG_PM, "Entering deep sleep again\n");
    esp_deep_sleep_start(); // Enter deep sleep
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/