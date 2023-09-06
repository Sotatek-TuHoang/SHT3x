/*****************************************************************************
 *
 * @file 	bee_nvs.c
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for save data, status into nvs flash
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include file                                                  ***/
/****************************************************************************/

#include "bee_nvs.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "stdint.h"

/****************************************************************************/
/***        Init Functions in App main                                    ***/
/****************************************************************************/

void nvs_flash_func_init()
{
    esp_err_t err = nvs_flash_init(); // Initialize NVS
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/