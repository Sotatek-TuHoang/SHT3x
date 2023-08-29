/*****************************************************************************
 *
 * @file 	bee_button.h
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for project cotrol by button
 *
 ***************************************************************************/

#ifndef BEE_BUTTON_H
#define BEE_BUTTON_H

#define ESP_INTR_FLAG_DEFAULT 0

/**
 * @brief Initializes the Wi-Fi provisioning button.
 *
 * This function sets up the GPIO and interrupt settings for the Wi-Fi provisioning button.
 * It installs the ISR service, sets up the ISR handler, and configures the GPIO settings
 * for the button pin. It also creates a queue to handle provisioning events.
 * @param gpio_num gpio set to provisioning wifi
 *
 * @note The WIFI_PROV_BUTTON GPIO pin and other related configurations should be defined
 *       before calling this function.
 */
void button_init(int gpio_num);

/**
 * @brief Task for handling button press events.
 *
 * This task monitors the button state and calculates the duration of button presses.
 * Depending on the duration of the button press, it takes different actions:
 * - If the button is held between 3 to 6 seconds, it enters Provisioning WiFi Mode.
 * - If the button is held for more than 6 seconds, it enters OTA Mode.
 * - Otherwise, the task exits without performing any action.
 */
void button_task(void* arg);

#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/