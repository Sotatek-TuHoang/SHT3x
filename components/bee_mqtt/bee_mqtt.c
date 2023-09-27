/***************************************************************************
* @file 	bee_mqtt.h
* @author 	tuha
* @date 	5 July 2023
* @brief	module for send data through mqtt
*       	and receive command from host main through mqtt
***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "mqtt_client.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "bee_mqtt.h"
#include "bee_ota.h"

extern bool bButton_task;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static bool bMQTT_connected = false;
static RTC_DATA_ATTR uint8_t u8trans_code = 0;

static char cMac_str[13];
static char cTopic_pub[64] = "VB/DMP/VBEEON/CUSTOM/SMH/DeviceID/telemetry";
static char cTopic_sub[64] = "VB/DMP/VBEEON/CUSTOM/SMH/DeviceID/Command";
static char rxBuffer_MQTT[800];
static QueueHandle_t mqtt_cmd_queue;

static const char *TAG_MQTT = "MQTT";

static esp_mqtt_client_handle_t client = NULL;

/****************************************************************************/
/***        Event Handler                                                 ***/
/****************************************************************************/

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
            bMQTT_connected = true;
            
            if (bButton_task)
            {
                snprintf(cTopic_sub, sizeof(cTopic_sub),"VB/DMP/VBEEON/CUSTOM/SMH/%s/Command", cMac_str);
                esp_mqtt_client_subscribe(client, cTopic_sub, 0);
                ESP_LOGI(TAG_MQTT, "Topic subscribe: %s\n", cTopic_sub);
            }
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            bMQTT_connected = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            if ((event->data) != NULL)
            {
                ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
                snprintf(rxBuffer_MQTT, event->data_len + 1, event->data);
                xQueueSend(mqtt_cmd_queue, &rxBuffer_MQTT, portMAX_DELAY);
            }

            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
    }
}

static void wait_MQTT_connect(uint16_t wait_max_ms)
{
    TickType_t start_time = xTaskGetTickCount();
    TickType_t wait_duration = 0;

    while (!bMQTT_connected && (wait_duration <= wait_max_ms))
    {
        wait_duration = xTaskGetTickCount() - start_time;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void mqtt_disconnect()
{
    esp_mqtt_client_disconnect(client);
}

void mqtt_func_init(void)
{
    /*Config mqtt client*/
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_ADDRESS_URI,
        .credentials.username = USERNAME,
        .credentials.authentication.password = PASSWORD
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    /* Get mac Address and set topic*/
    uint8_t u8mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, u8mac);
    snprintf(cMac_str, sizeof(cMac_str), "%02X%02X%02X%02X%02X%02X", u8mac[0], u8mac[1], u8mac[2], u8mac[3], u8mac[4], u8mac[5]);
    snprintf(cTopic_pub, sizeof(cTopic_pub), "VB/DMP/VBEEON/CUSTOM/SMH/%s/telemetry", cMac_str);

    ESP_LOGI(TAG_MQTT, "Topic publish: %s\n", cTopic_pub);

    mqtt_cmd_queue = xQueueCreate(2, sizeof(cJSON*));
}

void pub_data(float fTemp, float fHumi)
{
    cJSON *json_data = cJSON_CreateObject(); // Create a JSON object for the data
    cJSON_AddStringToObject(json_data, "thing_token", cMac_str);
    cJSON_AddStringToObject(json_data, "cmd_name", "Bee.data");
    cJSON *values = cJSON_AddObjectToObject(json_data, "values"); // Create a nested JSON object for the 'values' field
    cJSON_AddNumberToObject(values, "temperature", fTemp);
    cJSON_AddNumberToObject(values, "humidity", fHumi);
    cJSON_AddNumberToObject(json_data, "trans_code", u8trans_code++);
    
    char *json_str = cJSON_Print(json_data); // Convert the JSON object to a string
    wait_MQTT_connect(100);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, QoS_0, 0); // Publish the JSON string via MQTT
    cJSON_Delete(json_data);
    free(json_str);
}

void pub_warning(uint8_t u8Values, float fTemp, float fHumi)
{
    cJSON *json_warnings = cJSON_CreateObject();// Create a JSON object for the warning
    cJSON_AddStringToObject(json_warnings, "thing_token", cMac_str);
    cJSON_AddStringToObject(json_warnings, "cmd_name", "Bee.data");
    cJSON_AddStringToObject(json_warnings, "object_type", "Bee.warning");
    cJSON *json_values = cJSON_AddObjectToObject(json_warnings, "values");
    cJSON_AddNumberToObject(json_values, "warning_values", u8Values);
    cJSON_AddNumberToObject(json_values, "temperature", fTemp);
    cJSON_AddNumberToObject(json_values, "humidity", fHumi);
    cJSON_AddNumberToObject(json_warnings, "trans_code", u8trans_code++);

    char *json_str = cJSON_Print(json_warnings); // Convert the JSON object to a string
    wait_MQTT_connect(200);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, QoS_1, 0); // Publish the JSON string via MQTT
    cJSON_Delete(json_warnings);
    free(json_str);
}

void pub_keep_alive(void)
{
    cJSON *json_keep_alive = cJSON_CreateObject();
    cJSON_AddStringToObject(json_keep_alive, "thing_token", cMac_str);
    cJSON *json_values = cJSON_AddObjectToObject(json_keep_alive, "values");
    cJSON_AddStringToObject(json_values, "eventType", "refresh");
    cJSON_AddStringToObject(json_values, "status", "ONLINE");
    cJSON_AddNumberToObject(json_keep_alive, "trans_code", u8trans_code++);

    char *json_str = cJSON_Print(json_keep_alive);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, QoS_0, 0);

    cJSON_Delete(json_keep_alive);
    free(json_str);
}

void pub_ota_status(char *values)
{
    cJSON *json_ota_status = cJSON_CreateObject(); // Create a JSON object for the OTA status
    cJSON_AddStringToObject(json_ota_status, "thing_token", cMac_str);
    cJSON_AddStringToObject(json_ota_status, "enity_type", "module_sht3x");
    cJSON_AddStringToObject(json_ota_status, "cmd_name", "Bee_ota");
    cJSON_AddStringToObject(json_ota_status, "object_type", "Bee.ota_info");
    cJSON_AddStringToObject(json_ota_status, "status", values);
    cJSON_AddNumberToObject(json_ota_status, "trans_code", u8trans_code++);

    char *json_str = cJSON_Print(json_ota_status); // Convert the JSON object to a string
    wait_MQTT_connect(500);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, QoS_0, 0); // Publish the JSON string via MQTT
    cJSON_Delete(json_ota_status);
    free(json_str);
}

void rx_mqtt_ota_task(void *pvParameters)
{
    TickType_t xMaxWaitTime_MQTT = pdMS_TO_TICKS(15000); // Max time to wait mqtt from server to ota is 15 sec
    pub_ota_status("Check_ota");
    for (;;)
    {
        if (xQueueReceive(mqtt_cmd_queue, &rxBuffer_MQTT, xMaxWaitTime_MQTT)) // Wait for an MQTT message to be received
        {
            cJSON *root = cJSON_Parse(rxBuffer_MQTT); // Parse the received MQTT message as a JSON object
            if (root != NULL)
            {
                char *cThing_token = cJSON_GetObjectItemCaseSensitive(root, "thing_token")->valuestring;
                char *cEntity_type = cJSON_GetObjectItemCaseSensitive(root, "enity_type")->valuestring;
                char *cCmd_name = cJSON_GetObjectItemCaseSensitive(root, "cmd_name")->valuestring;
                char *cObject_type = cJSON_GetObjectItemCaseSensitive(root, "object_type")->valuestring;
                char *cUrl = cJSON_GetObjectItemCaseSensitive(root, "url")->valuestring;

                // Check if the received command is an OTA update
                if ((strcmp(cThing_token, cMac_str) == 0)       &&
                    (strcmp(cEntity_type, "module_sht3x") == 0) &&
                    (strcmp(cCmd_name, "Bee.ota") == 0)         &&
                    (strcmp(cObject_type, "Bee.ota_info") == 0) &&)
                {
                    start_ota(cUrl); // Perform OTA update
                }
                else 
                {
                    ESP_LOGI(TAG_MQTT, "WRONG CMD!!!");
                    esp_restart();
                }
                cJSON_Delete(root);
            }
        }
        else
        {
            ESP_LOGI(TAG_MQTT, "TIME OUT!!!");
            esp_restart();
        }
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/