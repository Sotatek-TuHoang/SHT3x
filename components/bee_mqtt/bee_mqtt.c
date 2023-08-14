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

#include "bee_sht3x.h"
#include "bee_mqtt.h"
#include "bee_nvs.h"

/****************************************************************************/
/***        Extern Variables                                              ***/
/****************************************************************************/

extern float fTemp;
extern float fHumi;
extern float fTemp_diff;
extern float fHumi_diff;
extern uint8_t u8error_cnt;
extern uint8_t u8max_error_cnt;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
 
static uint8_t u8warning_values;
static uint8_t u8tmp_warning_values;
static bool bRead_status    = 0;
static bool bTemp_threshold = 0;
static bool bHumi_threshold = 0;
static bool bTemp_diff      = 0;
static bool bHumi_diff      = 0;

static uint8_t u8mac[6];
static char cMac_str[13];
static char cTopic_pub[64] = "VB/DMP/VBEEON/CUSTOM/SMH/DeviceID/telemetry";
static char cTopic_sub[64] = "VB/DMP/VBEEON/CUSTOM/SMH/DeviceID/Command";
static uint8_t u8trans_code = 0;
static char rxBuffer_MQTT[500];

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;
static QueueHandle_t mqtt_cmd_queue;

static bool bMQTT_CONNECTED = false;

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
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            bMQTT_CONNECTED = true;
            esp_mqtt_client_subscribe(client, cTopic_sub, 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            bMQTT_CONNECTED = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            if (event->data != NULL)
            {
                ESP_LOGI(TAG, "MQTT_EVENT_DATA");
                snprintf(rxBuffer_MQTT, event->data_len + 1, event->data);
                xQueueSend(mqtt_cmd_queue, &rxBuffer_MQTT, portMAX_DELAY);
            }

            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
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
    esp_wifi_get_mac(ESP_IF_WIFI_STA, u8mac);
    snprintf(cMac_str, sizeof(cMac_str), "%02X%02X%02X%02X%02X%02X", u8mac[0], u8mac[1], u8mac[2], u8mac[3], u8mac[4], u8mac[5]);
    snprintf(cTopic_pub, sizeof(cTopic_pub), "VB/DMP/VBEEON/CUSTOM/SMH/%s/telemetry", cMac_str);
    snprintf(cTopic_sub, sizeof(cTopic_sub),"VB/DMP/VBEEON/CUSTOM/SMH/%s/Command", cMac_str);
    ESP_LOGI(TAG, "Topic publish: %s\n", cTopic_pub);
    ESP_LOGI(TAG, "Topic subscribe: %s\n", cTopic_sub);

    mqtt_cmd_queue = xQueueCreate(10, sizeof(cJSON*));
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

static void pub_data(const char *object, float values)
{
    if (bMQTT_CONNECTED == true)
    {
        cJSON *json_data = cJSON_CreateObject();
        cJSON_AddStringToObject(json_data, "thing_token", cMac_str);
        cJSON_AddStringToObject(json_data, "cmd_name", "Bee.data");
        cJSON_AddStringToObject(json_data, "object_type", object);
        cJSON_AddNumberToObject(json_data, "values", values);
        cJSON_AddNumberToObject(json_data, "trans_code", ++u8trans_code);

        char *json_str = cJSON_Print(json_data);
        esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, 1, 0);

        cJSON_Delete(json_data);
        free(json_str);
    }
}

static void send_warning(void)
{
    cJSON *json_warning = cJSON_CreateObject();
    cJSON_AddStringToObject(json_warning, "thing_token", cMac_str);
    cJSON_AddStringToObject(json_warning, "cmd_name", "Bee.data");
    cJSON_AddStringToObject(json_warning, "object_type", "bee_warning");
    cJSON_AddNumberToObject(json_warning, "values", u8warning_values);
    cJSON_AddNumberToObject(json_warning, "trans_code", ++u8trans_code);
    
    char *json_str = cJSON_Print(json_warning);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, 1, 0);

    cJSON_Delete(json_warning);
    free(json_str);
}

static void check_warning(void)
{
    if (u8error_cnt == u8max_error_cnt)
    {
        bRead_status = 1;
    }
    else if (u8error_cnt == 0)
    {
        bRead_status = 0;
        if (fTemp > TEMP_THRESHOLD)
        {
            bTemp_threshold = 1;
        }
        else
        {
            bTemp_threshold = 0;
        }

        if (fHumi > HUMI_THRESHOLD)
        {
            bHumi_threshold = 1;
        }
        else
        {
            bHumi_threshold = 0;
        }

        if (fTemp_diff > TEMP_DIFF_THRESHOLD)
        {
            bTemp_diff = 1;
        }
        else
        {
            bTemp_diff = 0;
        }

        if (fHumi_diff > HUMI_DIFF_THRESHOLD)
        {
            bHumi_diff = 1;
        }
        else
        {
            bHumi_diff = 0;
        }
    }
    
    u8tmp_warning_values = (bRead_status << 4) | (bTemp_threshold << 3) | (bHumi_threshold << 2) | (bTemp_diff << 1) | bHumi_diff;
    if (u8tmp_warning_values != u8warning_values)
    {
        u8warning_values = u8tmp_warning_values;
        send_warning();
    }
}

static void send_keep_alive(void)
{
    cJSON *json_keep_alive = cJSON_CreateObject();
    cJSON_AddStringToObject(json_keep_alive, "thing_token", cMac_str);
    cJSON *json_values = cJSON_AddObjectToObject(json_keep_alive, "values");
    cJSON_AddStringToObject(json_values, "eventType", "refresh");
    cJSON_AddStringToObject(json_values, "status", "ONLINE");
    cJSON_AddNumberToObject(json_keep_alive, "trans_code", ++u8trans_code);

    char *json_str = cJSON_Print(json_keep_alive);
    esp_mqtt_client_publish(client, cTopic_pub, json_str, 0, 1, 0);

    cJSON_Delete(json_keep_alive);
    free(json_str);
}

/****************************************************************************/
/***        Tasks                                                         ***/
/****************************************************************************/

void send_mqtt_data_task(void *pvParameters)
{
    TickType_t lt_send_data_mqtt = xTaskGetTickCount();
    TickType_t lt_send_keep_alive = xTaskGetTickCount();
    TickType_t interval_data_mqtt = pdMS_TO_TICKS(30000);
    TickType_t interval_keep_alive = pdMS_TO_TICKS(60000);

    for(;;)
    {
        if (bMQTT_CONNECTED)
        {
            if (((xTaskGetTickCount() - lt_send_data_mqtt) >= interval_data_mqtt))
            {
                lt_send_data_mqtt = xTaskGetTickCount();
                if (u8error_cnt == 0)
                {
                    pub_data("bee_temp", fTemp);
                    pub_data("bee_humi", fHumi);
                }

            }

            check_warning();

            if ((xTaskGetTickCount() - lt_send_keep_alive) >= interval_keep_alive)
            {
                lt_send_keep_alive = xTaskGetTickCount();
                send_keep_alive();
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void receive_mqtt_config_task(void *pvParameters)
{
    for (;;)
    {
        if (xQueueReceive(mqtt_cmd_queue, &rxBuffer_MQTT, portMAX_DELAY))
        {
            cJSON *root = cJSON_Parse(rxBuffer_MQTT);
            if (root != NULL)
            {
                char* cThing_token = cJSON_GetObjectItemCaseSensitive(root, "thing_token")->valuestring;
                char* cCmd_name = cJSON_GetObjectItemCaseSensitive(root, "cmd_name")->valuestring;
                char* cObject_type = cJSON_GetObjectItemCaseSensitive(root, "object_type")->valuestring;
                int values = cJSON_GetObjectItemCaseSensitive(root, "values")->valueint;
                int trans_code = cJSON_GetObjectItemCaseSensitive(root, "trans_code")->valueint;

                ESP_LOGI(TAG, "\n Received MQTT command:");
                ESP_LOGI(TAG, "thing_token: %s", cThing_token);
                ESP_LOGI(TAG, "cmd_name: %s", cCmd_name);
                ESP_LOGI(TAG, "object: %s", cObject_type);
                ESP_LOGI(TAG, "values: %d", values);
                ESP_LOGI(TAG, "trans_code: %d\n", trans_code);
                
                if ((strcmp(cCmd_name, "Bee.conf") == 0) && (strcmp(cObject_type, "data") == 0) && (values == 1))
                {
                    pub_data("bee_temp", fTemp);
                    ESP_LOGI (TAG, "cmd temp ok\n");
                }
                else if ((strcmp(cCmd_name, "Bee.conf") == 0) && (strcmp(cObject_type, "data") == 0) && (values == 2))
                {
                    pub_data("bee_humi", fHumi);
                    ESP_LOGI (TAG, "cmd humi ok\n");
                }
                else 
                {
                    ESP_LOGI (TAG, "wrong cmd\n");
                }
                cJSON_Delete(root);
            }
        }
    }
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/