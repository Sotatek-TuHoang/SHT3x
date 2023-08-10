/*****************************************************************************
 *
 * @file 	bee_wifi.c
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for provising and connect wifi by smart config (SoftAP)
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
#include <wifi_provisioning/scheme_softap.h>

#include "bee_wifi.h"
#include "bee_nvs.h"

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static const char *TAG = "Wifi prov";
const int WIFI_CONNECTED_EVENT = BIT0;

static bool bProv = false; 

static char cReceived_ssid[32];
static char cReceived_password[64];

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

                snprintf(cReceived_ssid, sizeof(cReceived_ssid), "%s", (const char *)wifi_sta_cfg->ssid);
                snprintf(cReceived_password, sizeof(cReceived_password), "%s", (const char *)wifi_sta_cfg->password);

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
                save_wifi_cred_to_nvs(cReceived_ssid, cReceived_password);
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
                wifi_prov_mgr_deinit();
                bProv = false;
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
                ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Connected!");
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
                break;
            default:
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

static void reconnect_old_wifi(void)
{
    wifi_config_t wifi_sta_cfg;

    // Đọc thông tin SSID và Password từ NVS
    char cSsid[32];
    char cPassword[64];

    load_old_wifi_cred(cSsid, cPassword);

    if (strlen(cSsid) > 0)
    {
        // Thiết lập cấu hình Wi-Fi với thông tin từ NVS
        memset(&wifi_sta_cfg, 0, sizeof(wifi_config_t));
        strncpy((char*)wifi_sta_cfg.sta.ssid, cSsid, sizeof(wifi_sta_cfg.sta.ssid));
        strncpy((char*)wifi_sta_cfg.sta.password, cPassword, sizeof(wifi_sta_cfg.sta.password));

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        // Khởi tạo Wi-Fi ở chế độ STA với cấu hình đã đọc từ NVS
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_cfg));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
}

void cnt_timeout(uint8_t *u8time)
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
/***        Event handler                                                 ***/
/****************************************************************************/
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
/***        initializing wifi function                                    ***/
/****************************************************************************/
/** @brief Hàm kiểm soát việc cài đặt cấu hình TCP/IP, đăng ký event handle
 *         Init các cấu hình liên quan phục vụ cho cấu hình cho wifi        */
void wifi_init_func(void)
{
    ESP_ERROR_CHECK(esp_netif_init()); /* Initialize TCP/IP */

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config =
    {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    /*Kiểm tra trạng thái cấu hình wifi đã tồn tại hay chưa*/
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    if (provisioned)
    {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        wifi_prov_mgr_deinit();
        wifi_init_sta();
    }
    else
    {
      wifi_prov_mgr_deinit();  
    }
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
}

/****************************************************************************/
/***        Global Functions                                              ***/
/****************************************************************************/
/** @brief Hàm kiểm soát việc cấu hình wifi */
void wifi_prov(void)
{
    if (!bProv)
    {
        bProv = true;

        /* Configuration for the provisioning manager */
        wifi_prov_mgr_config_t config =
        {
            .scheme = wifi_prov_scheme_softap,
            .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
        };

        ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = "Bee@1234"; /*Mật khẩu cho việc thực hiện cấu hình qua*/
        wifi_prov_security1_params_t *sec_params = pop;
        const char *service_key = NULL;
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
/** @brief Đếm thời gian tối đa cho việc cấu hình wifi là 60s
 * Hết thời gian tự động cấu hình lại bằng thông số wifi cũ */
void prov_timeout_task(void* pvParameters)
{
    uint8_t u8timeout_set = 60; // Đơn vị tính bằng giây
    cnt_timeout(&u8timeout_set);
    wifi_prov_mgr_stop_provisioning();
    reconnect_old_wifi();
    
    vTaskDelete(prov_timeout_handle); // Xóa task khi hoàn thành
}

/** @brief Đếm thời gian tối đa cho việc cấu hình wifi thất bại là 60s
*          Hết thời gian thì ngừng cấu hình*/
void prov_fail_task(void* pvParameters)
{
    wifi_prov_mgr_reset_sm_state_on_failure();

    uint8_t u8timeout_set = 60; // Đơn vị tính bằng giây
    cnt_timeout(&u8timeout_set);
    
    wifi_prov_mgr_stop_provisioning();

    reconnect_old_wifi();

    vTaskDelete(prov_fail_handle); // Xóa task khi hoàn thành
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
