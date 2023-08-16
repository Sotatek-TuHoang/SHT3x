#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"

#include "bee_deep_sleep.h"
#include "bee_mqtt.h"
#include "bee_sht3x.h"
#include "bee_i2c.h"

static RTC_DATA_ATTR struct timeval sleep_enter_time;

RTC_NOINIT_ATTR static uint8_t u8cnt_sleep = 0;

static const char *TAG_SHT3x = "SHT3x";

float fTemp;
float fHumi;
bool bSHT3x_status;

void deep_sleep_task(void *args)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_TIMER: {
            printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
            break;
        }

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset\n");
    }

    printf("Entering normal mode\n");


    sht3x_sensor_t* sensor;
    if ((sensor = sht3x_init_sensor (I2C_BUS, SHT3x_ADDR_1)))
    {
        vTaskDelay (sht3x_get_measurement_duration(sht3x_high));
        if (sht3x_measure (sensor, &fTemp, &fHumi))
        {
            bSHT3x_status = 0;
            ESP_LOGI (TAG_SHT3x, "Temperature: %.2f °C, Humidity: %.2f %%", fTemp, fHumi);
            pub_data("bee_temp", fTemp);
            pub_data("bee_humi", fHumi);
            check_warning();
        }
    }
    else 
    {
        bSHT3x_status = 1;
        check_warning();
        ESP_LOGI (TAG_SHT3x, "Can't read SHT3x");
    }

    ++u8cnt_sleep;
    printf ("u8cnt_sleep: %d\n", u8cnt_sleep);
    if (u8cnt_sleep >= 2)
    {
        u8cnt_sleep = 0;
        pub_keep_alive();
    }

    vTaskDelay(pdMS_TO_TICKS(1500));

    printf("Entering deep sleep again\n");

    // get deep sleep enter time
    gettimeofday(&sleep_enter_time, NULL);

    // enter deep sleep
    esp_deep_sleep_start();
}

void deep_sleep_register_rtc_timer_wakeup(void)
{
    const int wakeup_time_sec = 30;
    printf("Enabling timer wakeup, %ds\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
}

