#include "bee_sht3x.h"
#include "bee_i2c.h"
#include "bee_wifi.h"
#include "bee_mqtt.h"
#include "bee_nvs.h"
#include "bee_deep_sleep.h"
#include "bee_button.h"

TaskHandle_t sleep_task_handle = NULL;

void app_main(void)
{
    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);

    button_init(GPIO_NUM_2);

    xTaskCreate(wifi_prov_button_isr, "wifi_prov_button_isr", 4096, NULL, 3, NULL);
    
    nvs_flash_func_init();

    wifi_func_init();

    mqtt_func_init();

    deep_sleep_register_rtc_timer_wakeup();

    deep_sleep_register_ext1_wakeup(GPIO_NUM_2);

    xTaskCreate(deep_sleep_task, "deep_sleep_task", 4096, NULL, 19, &sleep_task_handle);
}
