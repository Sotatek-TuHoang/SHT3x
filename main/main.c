/***************************************************************************
* @file         main.c
* @author       tuha
* @date         14 August 2023
* @brief        Main application file for the Bee project
*               This file initializes various components, tasks, and sensors
*               to create a complete IoT system. It demonstrates the use of
*               the SHT3x temperature and humidity sensor, I2C communication,
*               Wi-Fi provisioning, MQTT communication, and non-volatile
*               storage (NVS). The application configures tasks for sensor
*               reading, Wi-Fi provisioning button handling, MQTT data
*               publishing, and MQTT configuration receiving.
****************************************************************************/
#include "bee_sht3x.h"
#include "bee_i2c.h"
#include "bee_wifi.h"
#include "bee_mqtt.h"
#include "bee_nvs.h"
#include "bee_button.h"


sht3x_sensor_t* sensor;    // sensor device data structure

void app_main(void)
{

    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    
    // Create the sensors, multiple sensors are possible.
    if ((sensor = sht3x_init_sensor (I2C_BUS, SHT3x_ADDR_1)))
    {
        // Create a user task that uses the sensors.
        xTaskCreate(&read_sht3x_task, "read_sht3x_task", 2048, sensor, 20, 0);
        printf("Create task SHT30 OK\n");
    }
    else
    {
        printf("Could not initialize SHT3x sensor\n");
    }

    button_init();

    xTaskCreate(wifi_prov_button_isr, "wifi_prov_button_isr", 4096, NULL, 3, NULL);

    nvs_flash_func_init();

    wifi_func_init();

    mqtt_func_init();

    xTaskCreate(send_mqtt_data_task, "send_mqtt_data_task", 4096, NULL, 7, NULL);

}
