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
#define H_TEMP_THRESHOLD       30
#define H_HUMI_THRESHOLD       80
#define L_TEMP_THRESHOLD       25
#define L_HUMI_THRESHOLD       60

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

void pub_data(const char *object, float values);

/**
 * @brief Checks sensor warnings and sends a warning message if conditions are met.
 * This function evaluates sensor error conditions and threshold crossings for temperature and humidity.
 * Depending on the conditions, it sets warning flags and triggers sending a warning message.
 * The warning values are composed into a byte, and if they differ from the previous state,
 * a warning message is sent using the `send_warning()` function.
 */
void check_warning();


/**
 * @brief Sends a keep-alive MQTT message to indicate device status.
 *
 * This function constructs a JSON message containing device status information such as the thing token,
 * event type, and status. The message is then published to the MQTT broker using the configured client.
 * The transmission code is also incremented for each message sent.
 *
 * @note The MQTT client (client) and topic (cTopic_pub) must be properly configured before calling this function.
 */
void pub_keep_alive(void);

#endif /* BEE_MQTT_H */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/