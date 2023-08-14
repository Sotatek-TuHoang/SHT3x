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
#define HUMI_DIFF_THRESHOLD  2

#define BROKER_ADDRESS_URI  "mqtt://61.28.238.97:1993"
#define USERNAME            "VBeeHome"
#define PASSWORD            "123abcA@!"

/**
 * @brief   Initialize MQTT functionality and configuration.
 *
 * This function configures the MQTT client with the provided broker address URI, username, and password. It initializes the
 * MQTT client, registers an event handler for MQTT events, and starts the MQTT client.
 * Additionally, the function retrieves the MAC address of the Wi-Fi station interface, constructs the MQTT topics using the
 * MAC address, and creates a message queue for handling MQTT commands.
 *
 * @note    The MQTT client is assumed to be a global variable accessible from the scope of this function.
 *
 * @param   None
 * @return  None
 */
void mqtt_func_init(void);

void send_mqtt_data_task(void* pvParameters);

void receive_mqtt_config_task(void* pvParameters);

#endif /* BEE_MQTT_H */

