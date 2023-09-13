/*****************************************************************************
 *
 * @file 	bee_wifi.c
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for provising and connect wifi by smart config (BLE)
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

#include "bee_wifi.h"
#include "bee_nvs.h"

extern bool bButton_task;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static const char *TAG = "Wifi";
const int WIFI_CONNECTED_EVENT = BIT0;

bool bProv = false; 

/****************************************************************************/
/***        List of handle                                      ***/
/****************************************************************************/
static EventGroupHandle_t wifi_event_group;
static TaskHandle_t prov_timeout_handle = NULL;
static TaskHandle_t prov_fail_handle = NULL;

/****************************************************************************/
/***        Event Handler                                                 ***/
/****************************************************************************/

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV:
            {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL:
            {   

                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                
                if (xTaskGetHandle("prov_timeout") != NULL)
                {
                    vTaskDelete(prov_timeout_handle);
                }
                if (xTaskGetHandle("prov_fail") != NULL)
                {
                    vTaskDelete(prov_fail_handle);
                }
                xTaskCreate(prov_fail_task, "prov_fail", 4096, NULL, 2, &prov_fail_handle);                
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");

                if (xTaskGetHandle("prov_timeout") != NULL)
                {
                    vTaskDelete(prov_timeout_handle);
                }
                if (xTaskGetHandle("prov_fail") != NULL)
                {
                    vTaskDelete(prov_fail_handle);
                }
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_stop_provisioning();
        
                bProv = false;
                bButton_task = false;
                break;
            default:
                break;
        }
    }
    else if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                break;
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    }
    else if (event_base == PROTOCOMM_TRANSPORT_BLE_EVENT)
    {
        switch (event_id)
        {
            case PROTOCOMM_TRANSPORT_BLE_CONNECTED:
                ESP_LOGI(TAG, "BLE transport: Connected!");
                break;
            case PROTOCOMM_TRANSPORT_BLE_DISCONNECTED:
                ESP_LOGI(TAG, "BLE transport: Disconnected!");
                break;
            default:
                break;
        }
    }
}

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf)
    {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */
    return ESP_OK;
}

/****************************************************************************/
/***        Locale Functions                                              ***/
/****************************************************************************/

static void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t u8eth_mac[6];
    const char *ssid_prefix = "BEE_";
    esp_wifi_get_mac(WIFI_IF_STA, u8eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, u8eth_mac[3], u8eth_mac[4], u8eth_mac[5]);
}

static void cnt_timeout(uint8_t *u8time)
{
    TickType_t prov_current_time = xTaskGetTickCount();
    TickType_t prov_timeout = pdMS_TO_TICKS(*u8time * 1000); // Timeout period, adjust as needed

    while ((xTaskGetTickCount() - prov_current_time) < prov_timeout)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG,"TIMEOUT!!!\n");
}

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void wifi_func_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init()); /* Initialize TCP/IP */

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config =
    {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    /*Kiểm tra trạng thái cấu hình wifi đã tồn tại hay chưa*/
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    if (provisioned)
    {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        wifi_prov_mgr_stop_provisioning();

        wifi_init_sta();

        const TickType_t xMaxDelay = pdMS_TO_TICKS(5000);
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, xMaxDelay);
    }
    else
    {
        wifi_prov_mgr_stop_provisioning();

    }
}

void wifi_prov(void)
{
    if (!bProv)
    {
        ESP_LOGI(TAG, "Start prov\n");
        bProv = true;

        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = "Bee@1234"; /*Mật khẩu cho việc thực hiện cấu hình qua*/
        wifi_prov_security1_params_t *sec_params = pop;
        const char *service_key = NULL;

        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
        };

        /* If your build fails with linker errors at this point, then you may have
         * forgotten to enable the BT stack or BTDM BLE settings in the SDK (e.g. see
         * the sdkconfig.defaults in the example project) */
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);

        wifi_prov_mgr_endpoint_create("custom-data");
        /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *) sec_params, service_name, service_key));
        wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
        /*Tạo task cho việc đếm thời gian cấu hình wifi tối đa*/
        xTaskCreate(prov_timeout_task, "prov_timeout", 4096, NULL, 1, &prov_timeout_handle);
    }
}

/****************************************************************************/
/***        Tasks                                                         ***/
/****************************************************************************/

void prov_timeout_task(void* pvParameters)
{
    uint8_t u8timeout_set = 60; // Đơn vị tính bằng giây
    cnt_timeout(&u8timeout_set);
    bButton_task = false;
    vTaskDelete(prov_timeout_handle); // Xóa task khi hoàn thành
}

void prov_fail_task(void* pvParameters)
{
    wifi_prov_mgr_reset_sm_state_on_failure();
    uint8_t u8timeout_set = 60; // Đơn vị tính bằng giây
    cnt_timeout(&u8timeout_set);
    bButton_task = false;
    vTaskDelete(prov_fail_handle); // Xóa task khi hoàn thành
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
