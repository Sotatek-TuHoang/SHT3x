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

void button_task(void* arg);

#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/