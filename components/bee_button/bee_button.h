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
 * @brief WiFi Provisioning Button ISR Handler
 *
 * This function is the Interrupt Service Routine (ISR) handler for the WiFi provisioning button.
 * It is executed when the WiFi provisioning button is clicked, and it triggers the WiFi provisioning process.
 *
 * @param arg - Pointer to any additional data that needs to be passed to the ISR (not used in this context).
 */
void wifi_prov_button_isr(void* arg);

#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/