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
#include "esp_log.h"

#include "bee_button.h"
#include "bee_wifi.h"
#include "bee_ota.h"
#include "bee_nvs.h"
#include "bee_mqtt.h"

/****************************************************************************/
/***        Global Variables                                              ***/
/****************************************************************************/
extern bool bInit;
bool bButton_task = false;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static bool button_pressed = false;
static TickType_t button_press_time = 0;
static TickType_t current_time = 0;
static const char *TAG = "BUTTON";

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    button_pressed = !button_pressed; // Toggle the button_pressed state
    if (button_pressed && !bButton_task) // Check if the button is pressed and no button_task is running
    {
        button_press_time = xTaskGetTickCount(); // Record the button press time
        xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL); // Create a new task for handling the button press
    }
}

void button_task(void* arg)
{
    TickType_t press_duration = 0;
    while (button_pressed && !bButton_task)
    {
        // Calculate the duration of the button press
        current_time = xTaskGetTickCount();
        press_duration = (current_time - button_press_time) * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Button pressed for %lu ms\n", (uint32_t)press_duration);
        if (press_duration >= 3000 && press_duration <= 6000)
        {

        }
        else
        {

        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    if (press_duration >= 3000 && press_duration <= 6000)
    {
        bButton_task = true;
        ESP_LOGI(TAG, "Enter Prov WiFi Mode\n");

        if (!bInit) //Check and init resource for Provisioning WiFi
        {
            nvs_flash_func_init();
            wifi_func_init();            
        }
        wifi_prov();
        vTaskDelete(NULL);
    }
    else if (press_duration > 6000)
    {
        bButton_task = true;
        ESP_LOGI(TAG, "Enter OTA Mode\n");

        if (!bInit) //Check and init resource for OTA
        {
            nvs_flash_func_init();
            wifi_func_init();     
            mqtt_func_init();       
        }
        xTaskCreate(rx_mqtt_ota_task, "rx_mqtt_ota_task", 8192, NULL, 9, NULL);
        vTaskDelete(NULL); 
    }
    else
    {
        vTaskDelete(NULL);
    }
}

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

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
