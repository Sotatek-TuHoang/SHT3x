/*****************************************************************************
 *
 * @file 	bee_mqtt.h
 * @author 	tuha
 * @date 	5 July 2023
* @brief	module for send data through mqtt
 * @brief	and receive command from host main through mqtt
 ***************************************************************************/

/****************************************************************************/
#include <stdint.h>
#ifndef BEE_MQTT_H
#define BEE_MQTT_H

#define MAC_ADDR_SIZE        6
#define TEMP_THRESHOLD       30
#define HUMI_THRESHOLD       80
#define TEMP_DIFF_THRESHOLD  2
#define HUMI_DIFF_THRESHOLD  5

#define BROKER_ADDRESS_URI  "mqtt://61.28.238.97:1993"
#define USERNAME            "VBeeHome"
#define PASSWORD            "123abcA@!"

void mqtt_app_start(void);

void send_mqtt_data_task(void* pvParameters);
//void receive_mqtt_config_task(void* pvParameters);

#endif /* BEE_MQTT_H */

