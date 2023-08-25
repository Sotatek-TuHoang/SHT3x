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

int extract_version(const char *filename) {
    int major;
    int minor;
    if (sscanf(filename, "SHT3x_Powersave_v%d.%d", &major, &minor) == 2) {
        return major * 100 + minor;
    }
    return -1;
}

const char *find_latest_version(const char *response) {
    const char *latest_version = NULL;
    int latest_version_number = -1;

    const char *token = strtok((char *)response, "\n");
    while (token != NULL) {
        int version = extract_version(token);
        if (version > latest_version_number) {
            latest_version_number = version;
            latest_version = token;
        }
        token = strtok(NULL, "\n");
    }

    return latest_version;
}

void start_ota_task(void *pvParameter)
{
    ESP_LOGI(TAG, "start ota task");
    esp_http_client_config_t list_config =
    {
        .url = SERVER_OTA_URL,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t list_client = esp_http_client_init(&list_config);
    esp_err_t list_ret = esp_http_client_perform(list_client);

    if (list_ret == ESP_OK)
    {
        int response_len = esp_http_client_get_content_length(list_client);
        if (response_len > 0)
        {
            char *response_buffer = malloc(response_len + 1);
            esp_http_client_read(list_client, response_buffer, response_len);
            response_buffer[response_len] = '\0';

            const char *latest_version = find_latest_version(response_buffer);

            if (latest_version)
            {
                char url[256];
                snprintf(url, sizeof(url), "%s%s", SERVER_OTA_URL, latest_version);

                esp_http_client_config_t config =
                {
                    .url = url,
                    .event_handler = _http_event_handler,
                    .cert_pem = (char *)server_cert_pem_start,
                    .skip_cert_common_name_check = true,
                };

                esp_https_ota_config_t ota_config =
                {
                    .http_config = &config,
                };

                ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
                esp_err_t ret = esp_https_ota(&ota_config);
                if (ret == ESP_OK)
                {
                    ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
                    esp_restart();
                } else
                {
                    ESP_LOGE(TAG, "Firmware upgrade failed");
                }
            }
            else
            {
                ESP_LOGE(TAG, "No OTA files found");
            }

            free(response_buffer);
        }
        else
        {
            ESP_LOGE(TAG, "Empty response");
        }
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed");
    }

    esp_http_client_cleanup(list_client);
    vTaskDelete(NULL);
}
