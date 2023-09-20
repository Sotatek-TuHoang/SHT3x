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
#include "esp_log.h"

static const char *TAG = "NVS";

/****************************************************************************/
/***        Exported functions                                            ***/
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

void save_wifi_cred_to_nvs(const char *cSsid, const char *cPassword, uint8_t u8channel)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_WIFI_CRED, NVS_READWRITE, &nvs_handle);

    if (err == ESP_OK)
    {
        err = nvs_set_str(nvs_handle, NVS_WIFI_SSID, cSsid);
        err |= nvs_set_str(nvs_handle, NVS_WIFI_PASS, cPassword);
        err |= nvs_set_u8(nvs_handle, NVS_WIFI_CHANNEL, u8channel);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Error saving wifi credentials to NVS! (%s)\n", esp_err_to_name(err));
        }

        nvs_close(nvs_handle);
    }
    else
    {
        ESP_LOGE(TAG, "Error opening NVS handle! (%s)\n", esp_err_to_name(err));
    }
}

void load_old_wifi_cred(char *cSsid, char *cPassword, uint8_t *u8channel)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_WIFI_CRED, NVS_READONLY, &nvs_handle);

    if (err == ESP_OK)
    {
        size_t ssid_len = 32;
        size_t password_len = 64;

        err = nvs_get_str(nvs_handle, NVS_WIFI_SSID, cSsid, &ssid_len);
        err |= nvs_get_str(nvs_handle, NVS_WIFI_PASS, cPassword, &password_len);
        err |= nvs_get_u8(nvs_handle, NVS_WIFI_CHANNEL, u8channel);

        if (err == ESP_OK)
        {
            // Process loaded data, if necessary
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Wifi cred not found in NVS\n");
        }
        else
        {
            ESP_LOGE(TAG, "Error reading wifi credentials from NVS! (%s)\n", esp_err_to_name(err));
        }

        nvs_close(nvs_handle);
    }
    else
    {
        ESP_LOGE(TAG, "Error opening NVS handle! (%s)\n", esp_err_to_name(err));
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/