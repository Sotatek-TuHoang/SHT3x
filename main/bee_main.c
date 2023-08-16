#include "bee_sht3x.h"
#include "bee_i2c.h"
#include "bee_wifi.h"
#include "bee_mqtt.h"
#include "bee_nvs.h"
#include "bee_button.h"
#include "bee_deep_sleep.h"

TaskHandle_t sleep_task_handle = NULL;

void app_main(void)
{
    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    
    // Create the sensors, multiple sensors are possible.

    button_init();

    xTaskCreate(wifi_prov_button_isr, "wifi_prov_button_isr", 4096, NULL, 3, NULL);

    nvs_flash_func_init();

    wifi_func_init();

    mqtt_func_init();

    deep_sleep_register_rtc_timer_wakeup();

    xTaskCreate(deep_sleep_task, "deep_sleep_task", 4096, NULL, 19, &sleep_task_handle);
}
