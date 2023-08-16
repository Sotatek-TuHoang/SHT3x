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

#define NVS_WARNING             "warn"
#define NVS_WARNING_VALUES      "warn_vals"

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
 * @brief   Save Wi-Fi credentials to Non-Volatile Storage (NVS).
 *
 * This function saves the provided Wi-Fi SSID and password to the NVS. It opens the NVS handle, stores the SSID and password
 * using appropriate keys, and then closes the NVS handle. Any errors encountered during the process are logged.
 *
 * @param   cSsid Wi-Fi SSID to be saved.
 * @param   cPassword Wi-Fi password to be saved.
 * @return  None
 */
void save_wifi_cred_to_nvs(const char *cSsid, const char *cPassword);

/**
 * @brief   Load previously saved Wi-Fi credentials from Non-Volatile Storage (NVS).
 *
 * This function reads the previously saved Wi-Fi SSID and password from the NVS. It opens the NVS handle, retrieves the SSID
 * and password using appropriate keys, and then closes the NVS handle. Any errors encountered during the process are logged.
 *
 * @param   cSsid Pointer to a character array to store the loaded Wi-Fi SSID.
 * @param   cPassword Pointer to a character array to store the loaded Wi-Fi password.
 * @return  None
 */
void load_old_wifi_cred(char *cSsid, char *cPassword);

#endif /* BEE_NVS_H */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/