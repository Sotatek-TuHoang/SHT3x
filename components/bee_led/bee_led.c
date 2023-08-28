#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bee_led.h"

void led_init(void)
{
    gpio_reset_pin(RED_LED);
    gpio_reset_pin(GREEN_LED);
    gpio_reset_pin(BLUE_LED);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(RED_LED, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GREEN_LED, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(BLUE_LED, GPIO_PULLUP_ONLY);

}

static void led_off()
{
    gpio_set_level(RED_LED, 0);
    gpio_set_level(GREEN_LED, 0);
    gpio_set_level(BLUE_LED, 0);
}

void led_blink(uint8_t gpio_pin, bool on_color)
{
    if (on_color)
    {
        gpio_set_level(gpio_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        led_off();
    }
}
