/*****************************************************************************
 *
 * @file 	bee_button.c
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for project cotrol by button
 *
 ***************************************************************************/

#include "bee_button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <esp_log.h>

#include "bee_button.h"
#include "bee_wifi.h"

static const char *TAG = "Button";

QueueHandle_t       wifi_prov_evt_queue;

static void IRAM_ATTR wifi_prov_button_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(wifi_prov_evt_queue, &gpio_num, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

void wifi_prov_button_isr(void* arg)
{
    uint8_t gpio_num;
    for (;;) {
        if (xQueueReceive(wifi_prov_evt_queue, &gpio_num, portMAX_DELAY))
        {
            wifi_prov();
            ESP_LOGI(TAG, "Wifi prov button clicked");
        }
    }
}


void button_init(int gpio_num)
{
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, wifi_prov_button_isr_handler, (void*) gpio_num);

    esp_rom_gpio_pad_select_gpio(gpio_num);

    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(gpio_num, GPIO_INTR_POSEDGE);

    wifi_prov_evt_queue = xQueueCreate(5, sizeof(uint8_t));
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/