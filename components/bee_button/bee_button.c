/*****************************************************************************
 *
 * @file 	bee_button.c
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for project cotrol by button
 *
 ***************************************************************************/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "bee_button.h"
#include "bee_wifi.h"
#include "bee_ota.h"
#include "bee_nvs.h"

static bool button_pressed = false;
static TickType_t button_press_time = 0;
static TickType_t current_time = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    button_pressed = !button_pressed;
    if (button_pressed)
    {
        button_press_time = xTaskGetTickCount();
        xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);
    }
}

void button_task(void* arg)
{
    while (button_pressed)
    {
        current_time = xTaskGetTickCount();
        TickType_t press_duration = (current_time - button_press_time) * portTICK_PERIOD_MS;
        printf("Button pressed for %lu ms\n", (uint32_t)press_duration);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    TickType_t press_duration = (current_time - button_press_time) * portTICK_PERIOD_MS;
    if (press_duration >= 3000 && press_duration <= 6000)
    {
        printf("Enter Prov WiFi Mode\n");
        nvs_flash_func_init();
        wifi_func_init();
        wifi_prov();
    }
    else if (press_duration > 6000)
    {
        nvs_flash_func_init();
        wifi_func_init();
        xTaskCreate (start_ota_task, "start_ota_task", 4096, NULL, 20, NULL );
        printf("Enter OTA Mode\n");
    }

    vTaskDelete(NULL);
}

void button_init(int gpio_num)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(gpio_num, gpio_isr_handler, NULL);
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
