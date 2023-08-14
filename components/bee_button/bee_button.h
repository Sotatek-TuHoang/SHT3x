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

#define WIFI_PROV_BUTTON        GPIO_NUM_0

/**
 * @brief Initializes the Wi-Fi provisioning button.
 *
 * This function sets up the GPIO and interrupt settings for the Wi-Fi provisioning button.
 * It installs the ISR service, sets up the ISR handler, and configures the GPIO settings
 * for the button pin. It also creates a queue to handle provisioning events.
 *
 * @note The WIFI_PROV_BUTTON GPIO pin and other related configurations should be defined
 *       before calling this function.
 *
 * @return None
 */
void button_init();

/**
 * @brief ISR handler for the Wi-Fi provisioning button.
 *
 * This ISR handler is triggered when the Wi-Fi provisioning button is clicked.
 * It receives events from the provisioning event queue and initiates the Wi-Fi provisioning process.
 * Additionally, it logs a message indicating that the button has been clicked.
 *
 * @param arg Pointer to the argument associated with the ISR handler (not used).
 *
 * @return None
 */
void wifi_prov_button_isr(void* arg);

#endif
