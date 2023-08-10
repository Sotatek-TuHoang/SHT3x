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
void nvs_flash_function_init()
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
/***        NVS Functions                                                 ***/
/****************************************************************************/
void save_wifi_cred_to_nvs(const char *cSsid, const char *cPassword)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Mở NVS
    err = nvs_open(NVS_WIFI_CRED, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS handle! (%s)\n", esp_err_to_name(err));
        return;
    }

    // Lưu ssid wifi vào NVS
    err = nvs_set_str(nvs_handle, NVS_WIFI_SSID, cSsid);
    if (err != ESP_OK)
    {
        printf("Error saving wifi SSID to NVS! (%s)\n", esp_err_to_name(err));
    }

    // Lưu password wifi vào NVS
    err = nvs_set_str(nvs_handle, NVS_WIFI_PASS, cPassword);
    if (err != ESP_OK)
    {
        printf("Error saving wifi password to NVS! (%s)\n", esp_err_to_name(err));
    }

    // Đóng NVS handle
    nvs_close(nvs_handle);
}

void load_old_wifi_cred(char *cSsid, char *cPassword)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Mở NVS
    err = nvs_open(NVS_WIFI_CRED, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS handle! (%s)\n", esp_err_to_name(err));
        return;
    }

    // Đọc SSID wifi từ NVS
    size_t ssid_len = 32;
    err = nvs_get_str(nvs_handle, NVS_WIFI_SSID, cSsid, &ssid_len);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        printf("Error reading old SSID wifi from NVS! (%s)\n", esp_err_to_name(err));
    }
    else if (err != ESP_OK)
    {
        printf("Error reading wifi SSID from NVS! (%s)\n", esp_err_to_name(err));
    }

    // Đọc password wifi từ NVS
    size_t password_len = 64;
    err = nvs_get_str(nvs_handle, NVS_WIFI_PASS, cPassword, &password_len);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        printf("Error reading old wifi password from NVS! (%s)\n", esp_err_to_name(err));
    }
    else if (err != ESP_OK)
    {
        printf("Error reading wifi password from NVS! (%s)\n", esp_err_to_name(err));
    }

    // Đóng NVS handle
    nvs_close(nvs_handle);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/