/*****************************************************************************
 *
 * @file    main.c
 * @author  tuha
 * @date    17 August 2023
 * @brief   Lib deep sleep functionality with WiFi configuration
 *
 *****************************************************************************/

// Include headers and libraries
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

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

// Function to send sensor data
static void send_data(void)
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
            pub_data("bee_temp", fTemp);
            pub_data("bee_humi", fHumi);
            check_warning();
        }
    }
    else 
    {
        bSHT3x_status = true;
        check_warning();
        ESP_LOGI(TAG_SHT3x, "Can't read SHT3x");
    }
}

// Function to send keep-alive message per 70s (2 times sleep)
static void send_keep_alive()
{
    ++u8cnt_sleep;
    if (u8cnt_sleep >= 2)
    {
        u8cnt_sleep = 0;
        pub_keep_alive();
    }
}

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void deep_sleep_register_rtc_timer_wakeup(void)
{
    const int wakeup_time_sec = 30;
    ESP_LOGI(TAG_PM, "Enabling timer wakeup, %ds\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
}

void deep_sleep_register_ext1_wakeup(int gpio_wakeup)
{
    const int ext_wakeup_pin = gpio_wakeup;
    const uint64_t ext_wakeup_pin_mask = 1ULL << ext_wakeup_pin;
    
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_mask, ESP_EXT1_WAKEUP_ANY_HIGH));
}

/****************************************************************************/
/***        Task                                                          ***/
/****************************************************************************/

void deep_sleep_task(void *args)
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
            break;
        }

        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (wakeup_pin_mask != 0) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                ESP_LOGI(TAG_PM, "Wake up from GPIO %d\n", pin);
                wifi_prov(); //if wake up by gpio num 2, start provisioning wifi
            } else {
                ESP_LOGI(TAG_PM, "Wake up from GPIO\n");
            }
            break;
        }

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            ESP_LOGI(TAG_PM, "Not a deep sleep reset\n");
    }

    // Handle different operational modes
    ESP_LOGI(TAG_PM, "Entering normal mode\n");

    extern bool bProv;

    if (bProv == true)
    {
        // If in provisioning mode, delay the task until WiFi provisioning is complete or a timeout occurs.
        // This ensures that the device remains in the provisioning state until the process is finished.
        while (bProv == true)
        {
            vTaskDelay(100);
        }        
    }
    else
    {
        // If not in provisioning mode, perform the following actions:
        send_data();         // Send temperature and humidity data using MQTT.
        send_keep_alive();   // Send a keep-alive message to maintain the MQTT connection.
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second before entering deep sleep.
    }


    // Prepare for deep sleep and enter
    ESP_LOGI(TAG_PM, "Entering deep sleep again\n");

    // Get deep sleep enter time
    gettimeofday(&sleep_enter_time, NULL);

    // Enable timer-based wake-up for the next sleep cycle
    deep_sleep_register_rtc_timer_wakeup();
    
    // Enter deep sleep
    esp_deep_sleep_start();
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/