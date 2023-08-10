/*****************************************************************************
 *
 * @file 	bee_nvs.h
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for save data, status into nvs flash
 *
 ***************************************************************************/

/****************************************************************************/
#ifndef BEE_NVS_H_
#define BEE_NVS_H_

#include <stdbool.h>
#include <stdint.h>

#define NVS_DATA_CONFIG         "data_config"
#define NVS_SEND_DATA           "send_data"
#define NVS_UART_INTERVAL       "uart_interval"
#define NVS_MQTT_INTERVAL       "mqtt_interval"
#define NVS_PROVISIONED         "provisioned"

#define NVS_WIFI_CRED           "wifi_cred"
#define NVS_WIFI_PASS           "wifi_pass"
#define NVS_WIFI_SSID           "wifi_ssid"

void nvs_flash_function_init();
void save_uart_data_to_nvs(bool bsend_data, uint8_t u8data_interval_uart);
void load_data_from_nvs(bool* bsend_data, uint8_t* u8data_interval_uart, uint8_t* u8data_interval_mqtt);
void save_mqtt_data_to_nvs(uint8_t u8data_interval_mqtt);

void save_wifi_cred_to_nvs(const char *cSsid, const char *cPassword);
void load_old_wifi_cred(char *cSsid, char *cPassword);

#endif /* BEE_NVS_H */
