#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "bee_i2c.h"
#include "bee_sht3x.h"

#define SHT3x_STATUS_CMD               0xF32D
#define SHT3x_CLEAR_STATUS_CMD         0x3041
#define SHT3x_RESET_CMD                0x30A2
#define SHT3x_FETCH_DATA_CMD           0xE000
#define SHT3x_HEATER_OFF_CMD           0x3066

const uint16_t SHT3x_MEASURE_CMD[6][3] = { 
        {0x2400,0x240b,0x2416},    // [SINGLE_SHOT][H,M,L] without clock stretching
        {0x2032,0x2024,0x202f},    // [PERIODIC_05][H,M,L]
        {0x2130,0x2126,0x212d},    // [PERIODIC_1 ][H,M,L]
        {0x2236,0x2220,0x222b},    // [PERIODIC_2 ][H,M,L]
        {0x2234,0x2322,0x2329},    // [PERIODIC_4 ][H,M,L]
        {0x2737,0x2721,0x272a} };  // [PERIODIC_10][H,M,L]

// due to the fact that ticks can be smaller than portTICK_PERIOD_MS, one and
// a half tick period added to the duration to be sure that waiting time for 
// the results is long enough
#define TIME_TO_TICKS(ms) (1 + ((ms) + (portTICK_PERIOD_MS-1) + portTICK_PERIOD_MS/2 ) / portTICK_PERIOD_MS)

#define SHT3x_MEAS_DURATION_REP_HIGH   15
#define SHT3x_MEAS_DURATION_REP_MEDIUM 6
#define SHT3x_MEAS_DURATION_REP_LOW    4

// measurement durations in us 
const uint16_t SHT3x_MEAS_DURATION_US[3] = { SHT3x_MEAS_DURATION_REP_HIGH   * 1000, 
                                             SHT3x_MEAS_DURATION_REP_MEDIUM * 1000, 
                                             SHT3x_MEAS_DURATION_REP_LOW    * 1000 };

// measurement durations in RTOS ticks
const uint8_t SHT3x_MEAS_DURATION_TICKS[3] = { TIME_TO_TICKS(SHT3x_MEAS_DURATION_REP_HIGH), 
                                               TIME_TO_TICKS(SHT3x_MEAS_DURATION_REP_MEDIUM), 
                                               TIME_TO_TICKS(SHT3x_MEAS_DURATION_REP_LOW) };

#if defined(SHT3x_DEBUG_LEVEL_2)
#define debug(s, f, ...) printf("%s %s: " s "\n", "SHT3x", f, ## __VA_ARGS__)
#define debug_device(s, f, d, ...) printf("%s %s: bus %d, addr %02x - " s "\n", "SHT3x", f, d->bus, d->addr, ## __VA_ARGS__)
#else
#define debug(s, f, ...)
#define debug_device(s, f, d, ...)
#endif

#if defined(SHT3x_DEBUG_LEVEL_1) || defined(SHT3x_DEBUG_LEVEL_2)
#define error(s, f, ...) printf("%s %s: " s "\n", "SHT3x", f, ## __VA_ARGS__)
#define error_device(s, f, d, ...) printf("%s %s: bus %d, addr %02x - " s "\n", "SHT3x", f, d->bus, d->addr, ## __VA_ARGS__)

#else
#define error(s, f, ...)
#define error_device(s, f, d, ...)
#endif

/** Forward declaration of function for internal use */

static bool sht3x_is_measuring  (sht3x_sensor_t*);
static bool sht3x_send_command  (sht3x_sensor_t*, uint16_t);
static bool sht3x_read_data     (sht3x_sensor_t*, uint8_t*,  uint32_t);
static bool sht3x_get_status    (sht3x_sensor_t*, uint16_t*);
static bool sht3x_reset         (sht3x_sensor_t*);

static uint8_t crc8 (uint8_t data[], int len);

/** ------------------------------------------------ */

sht3x_sensor_t* sht3x_init_sensor(uint8_t bus, uint8_t addr)
{
    sht3x_sensor_t* device;

    if ((device = malloc (sizeof(sht3x_sensor_t))) == NULL)
        return NULL;
    
    // inititalize sensor data structure
    device->bus  = bus;
    device->addr = addr;
    device->mode = sht3x_single_shot;
    device->meas_start_time = 0;
    device->meas_started = false;
    device->meas_first = false;

    uint16_t status;

    // try to reset the sensor
    if (!sht3x_reset(device))
    {
        debug_device ("could not reset the sensor", __FUNCTION__, device);
    }
    
    // check again the status after clear status command
    if (!sht3x_get_status(device, &status))
    {
        error_device ("could not get sensor status", __FUNCTION__, device);
        free(device);
        return NULL;       
    }
    
    debug_device ("sensor initialized", __FUNCTION__, device);
    return device;
}


bool sht3x_measure (sht3x_sensor_t* device, float* fTemp, float* fHumi)
{
    if (!device || (!fTemp && !fHumi)) return false;

    if (!sht3x_start_measurement (device, sht3x_single_shot, sht3x_high))
        return false;

    vTaskDelay (SHT3x_MEAS_DURATION_TICKS[sht3x_high]);

    sht3x_raw_data_t raw_data;
    
    if (!sht3x_get_raw_data (device, raw_data))
        return false;
        
    return sht3x_compute_values (raw_data, fTemp, fHumi);
}


bool sht3x_start_measurement (sht3x_sensor_t* device, sht3x_mode_t mode, sht3x_repeat_t repeat)
{
    if (!device) return false;
    
    device->error_code = SHT3x_OK;
    device->mode = mode;
    device->repeatability = repeat;
    
    // start measurement according to selected mode and return an duration estimate
    if (!sht3x_send_command(device, SHT3x_MEASURE_CMD[mode][repeat]))
    {
        error_device ("could not send start measurment command", __FUNCTION__, device);
        device->error_code |= SHT3x_SEND_MEAS_CMD_FAILED;
        return false;
    }

    device->meas_start_time = sdk_system_get_time ();
    device->meas_started = true;
    device->meas_first = true;

    return true;
}


uint8_t sht3x_get_measurement_duration (sht3x_repeat_t repeat)
{
    return SHT3x_MEAS_DURATION_TICKS[repeat];  // in RTOS ticks
}


bool sht3x_get_raw_data(sht3x_sensor_t* device, sht3x_raw_data_t raw_data)
{
    if (!device || !raw_data) return false;

    device->error_code = SHT3x_OK;

    if (!device->meas_started)
    {
        debug_device ("measurement is not started", __FUNCTION__, device);
        device->error_code = SHT3x_MEAS_NOT_STARTED;
        return sht3x_is_measuring (device);
    }

    if (sht3x_is_measuring(device))
    {
        error_device ("measurement is still running", __FUNCTION__, device);
        device->error_code = SHT3x_MEAS_STILL_RUNNING;
        return false;
    }

    // send fetch command in any periodic mode (mode > 0) before read raw data
    if (device->mode && !sht3x_send_command(device, SHT3x_FETCH_DATA_CMD))
    {
        debug_device ("send fetch command failed", __FUNCTION__, device);
        device->error_code |= SHT3x_SEND_FETCH_CMD_FAILED;
        return false;
    }

    // read raw data
    if (!sht3x_read_data(device, raw_data, sizeof(sht3x_raw_data_t)))
    {
        error_device ("read raw data failed", __FUNCTION__, device);
        device->error_code |= SHT3x_READ_RAW_DATA_FAILED;
        return false;
    }

    // reset first measurement flag
    device->meas_first = false;
    
    // reset measurement started flag in single shot mode
    if (device->mode == sht3x_single_shot)
        device->meas_started = false;
    
    // check temperature crc
    if (crc8(raw_data,2) != raw_data[2])
    {
        error_device ("CRC check for fTemp data failed", __FUNCTION__, device);
        device->error_code |= SHT3x_WRONG_CRC_TEMPERATURE;
        return false;
    }

    // check humidity crc
    if (crc8(raw_data+3,2) != raw_data[5])
    {
        error_device ("CRC check for fHumi data failed", __FUNCTION__, device);
        device->error_code |= SHT3x_WRONG_CRC_HUMIDITY;
        return false;
    }

    return true;
}


bool sht3x_compute_values (sht3x_raw_data_t raw_data, float* fTemp, float* fHumi)
{
    if (!raw_data) return false;

    if (fTemp) 
        *fTemp = ((((raw_data[0] * 256.0) + raw_data[1]) * 175) / 65535.0) - 45;

    if (fHumi)
        *fHumi = ((((raw_data[3] * 256.0) + raw_data[4]) * 100) / 65535.0);
  
    return true;    
}


bool sht3x_get_results (sht3x_sensor_t* device, float* fTemp, float* fHumi)
{
    if (!device || (!fTemp && !fHumi)) return false;

    sht3x_raw_data_t raw_data;
    
    if (!sht3x_get_raw_data (device, raw_data))
        return false;
        
    return sht3x_compute_values (raw_data, fTemp, fHumi);
}

/* Functions for internal use only */

static bool sht3x_is_measuring (sht3x_sensor_t* device)
{
    if (!device) return false;

    device->error_code = SHT3x_OK;

    // not running if measurement is not started at all or 
    // it is not the first measurement in periodic mode
    if (!device->meas_started || !device->meas_first)
      return false;
    
    // not running if time elapsed is greater than duration
    uint32_t elapsed = sdk_system_get_time() - device->meas_start_time;

    return elapsed < SHT3x_MEAS_DURATION_US[device->repeatability];
}

static bool sht3x_send_command(sht3x_sensor_t* device, uint16_t cmd)
{
    if (!device) return false;

    uint8_t data[2] = { cmd >> 8, cmd & 0xff };

    debug_device ("send command MSB=%02x LSB=%02x", __FUNCTION__, device, data[0], data[1]);

    int err = i2c_slave_write(device->bus, device->addr, 0, data, 2);
  
    if (err)
    {
        device->error_code |= (err == -EBUSY) ? SHT3x_I2C_BUSY : SHT3x_I2C_SEND_CMD_FAILED;
        error_device ("i2c error %d on write command %02x", __FUNCTION__, device, err, cmd);
        return false;
    }

    return true;
}

static bool sht3x_read_data(sht3x_sensor_t* device, uint8_t *data,  uint32_t len)
{
    if (!device) return false;
    int err = i2c_slave_read(device->bus, device->addr, 0, data, len);
        
    if (err)
    {
        device->error_code |= (err == -EBUSY) ? SHT3x_I2C_BUSY : SHT3x_I2C_READ_FAILED;
        error_device ("error %d on read %lu byte", __FUNCTION__, device, err, (unsigned long)len);
        return false;
    }

#   ifdef SHT3x_DEBUG_LEVEL_2
    printf("SHT3x %s: bus %d, addr %02x - read following bytes: ", 
           __FUNCTION__, device->bus, device->addr);
    for (int i=0; i < len; i++)
        printf("%02x ", data[i]);
    printf("\n");
#   endif // ifdef SHT3x_DEBUG_LEVEL_2

    return true;
}

static bool sht3x_reset (sht3x_sensor_t* device)
{
    if (!device) return false;

    debug_device ("soft-reset triggered", __FUNCTION__, device);
    
    device->error_code = SHT3x_OK;

    // send reset command
    if (!sht3x_send_command(device, SHT3x_RESET_CMD))
    {
        device->error_code |= SHT3x_SEND_RESET_CMD_FAILED;
        return false;
    }   
    // wait for small amount of time needed (according to datasheet 0.5ms)
    vTaskDelay (100 / portTICK_PERIOD_MS);
    
    uint16_t status;

    // check the status after reset
    if (!sht3x_get_status(device, &status))
        return false;
        
    return true;    
}

static bool sht3x_get_status (sht3x_sensor_t* device, uint16_t* status)
{
    if (!device || !status) return false;

    device->error_code = SHT3x_OK;

    uint8_t  data[3];

    if (!sht3x_send_command(device, SHT3x_STATUS_CMD) || !sht3x_read_data(device, data, 3))
    {
        device->error_code |= SHT3x_SEND_STATUS_CMD_FAILED;
        return false;
    }

    *status = data[0] << 8 | data[1];
    debug_device ("status=%02x", __FUNCTION__, device, *status);
    return true;
}


const uint8_t g_polynom = 0x31;

static uint8_t crc8 (uint8_t data[], int len)
{
    // initialization value
    uint8_t crc = 0xff;
    
    // iterate over all bytes
    for (int i=0; i < len; i++)
    {
        crc ^= data[i];  
    
        for (int i = 0; i < 8; i++)
        {
            bool xor = crc & 0x80;
            crc = crc << 1;
            crc = xor ? crc ^ g_polynom : crc;
        }
    }

    return crc;
} 

void read_sht3x_task (void *pvParameters)
{
    sht3x_sensor_t* sensor = (sht3x_sensor_t*)pvParameters;
    float fTemp;
    float fHumi;

    vTaskDelay (sht3x_get_measurement_duration(sht3x_high));

    TickType_t last_wakeup = xTaskGetTickCount();
    
    for(;;) 
    {
        if ((xTaskGetTickCount() - last_wakeup) > pdMS_TO_TICKS(2000))
        {
            last_wakeup = xTaskGetTickCount();
            
            if (sht3x_measure (sensor, &fTemp, &fHumi))
            {
                printf("SHT3x Sensor: %.2f °C, %.2f %%\n", fTemp, fHumi);
            }
            else
            {

            }

        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
