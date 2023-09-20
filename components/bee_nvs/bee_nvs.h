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

#define NVS_WIFI_CRED           "wifi_cred"
#define NVS_WIFI_PASS           "wifi_pass"
#define NVS_WIFI_SSID           "wifi_ssid"
#define NVS_WIFI_CHANNEL        "wifi_channel"

/**
 * @brief   Initialize the Non-Volatile Storage (NVS) flash memory.
 *
 * This function initializes the NVS flash memory. If there are no free pages or a new version is found, it erases the NVS
 * and then attempts to initialize it again. Any errors encountered during initialization are checked and handled.
 *
 * @note    The function assumes that NVS flash memory initialization is crucial for proper system operation.
 *
 * @param   None
 * @return  None
 */
void nvs_flash_func_init();

/**
 * @brief Save Wi-Fi credentials to NVS.
 *
 * This function saves the provided Wi-Fi SSID and password to the NVS (Non-Volatile Storage).
 *
 * @param cSsid     Pointer to the Wi-Fi SSID string.
 * @param cPassword Pointer to the Wi-Fi password string.
 */
void save_wifi_cred_to_nvs(const char *cSsid, const char *cPassword, uint8_t u8channel);

/**
 * @brief Load old Wi-Fi credentials from NVS.
 *
 * This function loads the old Wi-Fi SSID and password from the Non-Volatile Storage (NVS).
 * It first opens the NVS, then reads the SSID and password values if they exist.
 * If not found or if any error occurs, appropriate error messages are printed.
 * The NVS handle is then closed.
 *
 * @param cSsid Buffer to store the loaded Wi-Fi SSID.
 * @param cPassword Buffer to store the loaded Wi-Fi password.
 */
void load_old_wifi_cred(char *cSsid, char *cPassword, uint8_t *u8channel);

#endif /* BEE_NVS_H */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/