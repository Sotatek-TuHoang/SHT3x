/*****************************************************************************
 *
 * @file 	bee_ota.c
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for update ota
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "bee_ota.h"
#include "bee_nvs.h"
#include "bee_wifi.h"
#include "bee_mqtt.h"

static const char *TAG = "OTA";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

/****************************************************************************/
/***        Exported Function                                             ***/
/****************************************************************************/

void start_ota(char *cUrl)
{
    ESP_LOGI(TAG, "Starting OTA task");
    pub_ota_status(VERSION);

    // Configure the HTTP client for OTA update
    esp_http_client_config_t config =
    {
        .url = cUrl,
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
    };

    config.skip_cert_common_name_check = true; // Skip common name check for server certificate

    // Initiate OTA update
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK)
    {
        pub_ota_status("Succeed"); // Publish OTA status as "Succeed"
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        esp_restart();
    }
    else
    {
        pub_ota_status("Failed"); // Publish OTA status as "Failed"
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGE(TAG, "Firmware upgrade failed");
        esp_restart();
    }
}

/****************************************************************************/
/***        End of file                                                   ***/
/****************************************************************************/
