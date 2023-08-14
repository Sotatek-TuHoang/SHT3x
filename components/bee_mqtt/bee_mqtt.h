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
 * @brief   Initialize MQTT functionality.
 *
 * This function configures and initializes the MQTT client. It sets up the MQTT broker address URI, credentials (username
 * and password), and initializes the MQTT client. It registers an event handler for MQTT events, starts the MQTT client, and
 * creates an MQTT command queue. The MAC address of the Wi-Fi station interface is retrieved to construct MQTT topics for
 * publishing and subscribing. Information about the topics and initialization progress is logged.
 *
 * @note    Make sure to customize the BROKER_ADDRESS_URI, USERNAME, and PASSWORD constants before using this function.
 */
void mqtt_func_init(void);

/**
 * @brief   Task to send MQTT data and keep-alive messages.
 *
 * This task is responsible for periodically sending MQTT data (temperature and humidity) and keep-alive messages to the broker.
 * It checks if the MQTT client is connected, and if so, sends the data and keep-alive messages based on the configured intervals.
 * It also handles a warning check. If the MQTT client is not connected, it waits for a brief period before checking again.
 *
 * @note    Ensure that the intervals (interval_data_mqtt and interval_keep_alive) are adjusted as needed for your application.
 * @note    The functions pub_data() and send_keep_alive() are assumed to be implemented elsewhere in the code.
 *
 * @param   pvParameters Pointer to task parameters (unused in this context).
 */
void send_mqtt_data_task(void* pvParameters);


/**
 * @brief   Task to receive and process MQTT configuration commands.
 *
 * This task listens for incoming messages from the MQTT command queue and processes the received JSON data. It parses the JSON
 * data to extract relevant information such as thing_token, cmd_name, object_type, values, and trans_code. It then logs the
 * received command details and takes appropriate actions based on the received data. If the command corresponds to a specific
 * condition, it may publish relevant data using the pub_data function. If the command does not match any expected conditions,
 * it logs a message indicating a wrong command.
 *
 * @note    The pub_data function is assumed to be implemented elsewhere in the code.
 *
 * @param   pvParameters Pointer to task parameters (unused in this context).
 */
void receive_mqtt_config_task(void* pvParameters);

#endif /* BEE_MQTT_H */

