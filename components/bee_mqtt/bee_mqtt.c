/***************************************************************************
* @file 	bee_mqtt.h
* @author 	tuha
* @date 	5 July 2023
* @brief	module for send data through mqtt
* @brief	and receive command from host main through mqtt
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

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static RTC_DATA_ATTR uint8_t u8trans_code = 0;

static char cMac_str[13];
static char cTopic_pub[64] = "VB/DMP/VBEEON/CUSTOM/SMH/DeviceID/telemetry";

static const char *TAG_MQTT = "MQTT";

static esp_mqtt_client_handle_t client = NULL;

/****************************************************************************/
/***        Event Handler                                                 ***/
/****************************************************************************/

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
    }
}

/****************************************************************************/
/***        Init Functions in App main                                    ***/
/****************************************************************************/

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
}

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void pub_data(float fTemp, float fHumi)
{
    cJSON *json_data = cJSON_CreateObject();
    cJSON_AddStringToObject(json_data, "thing_token", cMac_str);
    cJSON_AddStringToObject(json_data, "cmd_name", "Bee.data");
    cJSON *values = cJSON_AddObjectToObject(json_data, "values");
    cJSON_AddNumberToObject(values, "temperature", fTemp);
    cJSON_AddNumberToObject(values, "humidity", fHumi);
    
    cJSON_AddNumberToObject(json_data, "trans_code", u8trans_code++);

    char *json_str = cJSON_Print(json_data);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, 1, 0);

    cJSON_Delete(json_data);
    free(json_str);
}

void pub_warning(uint8_t u8Values)
{

    cJSON *json_data = cJSON_CreateObject();
    cJSON_AddStringToObject(json_data, "thing_token", cMac_str);
    cJSON_AddStringToObject(json_data, "cmd_name", "Bee.data");
    cJSON_AddStringToObject(json_data, "object_type", "Bee.warning");
    cJSON_AddNumberToObject(json_data, "values", u8Values);
    cJSON_AddNumberToObject(json_data, "trans_code", u8trans_code++);

    char *json_str = cJSON_Print(json_data);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, 1, 0);

    cJSON_Delete(json_data);
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
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, 1, 0);

    cJSON_Delete(json_keep_alive);
    free(json_str);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/