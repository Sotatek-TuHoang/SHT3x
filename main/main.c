#include "bee_sht3x.h"
#include "bee_i2c.h"

sht3x_sensor_t* sensor;    // sensor device data structure

void app_main(void)
{

    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    
    // Create the sensors, multiple sensors are possible.
    if ((sensor = sht3x_init_sensor (I2C_BUS, SHT3x_ADDR_1)))
    {
        // Create a user task that uses the sensors.
        xTaskCreate(read_sht3x_task, "read_sht3x_task", 2048, sensor, 2, 0);
        printf("Create task SHT30 OK\n");
    }
    else
    {
        printf("Could not initialize SHT3x sensor\n");
    }
}
