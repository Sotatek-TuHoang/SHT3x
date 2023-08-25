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

#include "bee_nvs.h"
#include "bee_deep_sleep.h"
#include "bee_mqtt.h"
#include "bee_sht3x.h"
#include "bee_i2c.h"
#include "bee_wifi.h"

/****************************************************************************/
/***        Static Variables                                              ***/
/****************************************************************************/
// storage variables to rtc memory, so variables dont reset after wake up from deep sleep
static RTC_DATA_ATTR struct timeval sleep_enter_time; 
static RTC_DATA_ATTR uint8_t u8cnt_sleep = 0;

// Define tags for log messages
static const char *TAG_SHT3x = "SHT3x";
static const char *TAG_PM = "POWER MODE";

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

float fTemp;
float fHumi;
bool bSHT3x_status;
static bool bInit;

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
    uint8_t u8Warning_value = check_warning(bSHT3x_status, fTemp, fHumi);
    if (u8Warning_value != NO_WARNNG)
    {
        init_resource_pub_mqtt();
        pub_warning(u8Warning_value);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Function to send sensor data
static void read_data(void)
{
    // Initialize and measure using the SHT3x sensor
    sht3x_sensor_t* sensor;
    if ((sensor = sht3x_init_sensor (I2C_BUS, SHT3x_ADDR_1)))
    {
        vTaskDelay (sht3x_get_measurement_duration(sht3x_high));
        if (sht3x_measure (sensor, &fTemp, &fHumi))
        {
            bSHT3x_status = false;
            ESP_LOGI(TAG_SHT3x, "Temperature: %.2f °C, Humidity: %.2f %%", fTemp, fHumi);
            check_and_pub_warning();
        }
        else
        {
            bSHT3x_status = true;
            check_and_pub_warning();
            ESP_LOGI(TAG_SHT3x, "Can't measure SHT3x");
        }
    }
    else 
    {
        bSHT3x_status = true;
        check_and_pub_warning();
        ESP_LOGI(TAG_SHT3x, "Can't init SHT3x");
    }
}

static void check_cause_wake_up(void)
{
    // Get current time and calculate sleep time
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    // Handle different wake-up causes
    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_TIMER:
        {
            ESP_LOGI(TAG_PM, "Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);

            if (u8cnt_sleep == 6)
            {
                init_resource_pub_mqtt();
                read_data();
                pub_data(fTemp, fHumi);
                pub_keep_alive();
                u8cnt_sleep = 0;
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            else if (u8cnt_sleep == 3)
            {
                u8cnt_sleep++;
                init_resource_pub_mqtt();
                read_data();
                pub_data(fTemp, fHumi);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            else
            {   
                u8cnt_sleep++;
                read_data();
            }
            break;
        }

        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (wakeup_pin_mask != 0)
            {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                ESP_LOGI(TAG_PM, "Wake up from GPIO %d\n", pin);

                nvs_flash_func_init();
                wifi_func_init();
                wifi_prov();

                vTaskDelay (20000 / portTICK_PERIOD_MS);
            }
            else
            {
                ESP_LOGI(TAG_PM, "Wake up from GPIO\n");
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

void deep_sleep_register_ext1_wakeup(int gpio_wakeup)
{
    const int ext_wakeup_pin = gpio_wakeup;
    const uint64_t ext_wakeup_pin_mask = 1ULL << ext_wakeup_pin;
    
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_mask, ESP_EXT1_WAKEUP_ANY_HIGH));

    ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(ext_wakeup_pin));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(ext_wakeup_pin));
}

/****************************************************************************/
/***        Task                                                          ***/
/****************************************************************************/

void deep_sleep_task(void *args)
{    
    // Handle different operational modes
    ESP_LOGI(TAG_PM, "Entering normal mode\n");

    check_cause_wake_up();

    // Prepare for deep sleep and enter
    ESP_LOGI(TAG_PM, "Entering deep sleep again\n");

    // Get deep sleep enter time
    gettimeofday(&sleep_enter_time, NULL);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/