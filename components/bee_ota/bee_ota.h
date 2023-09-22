/*****************************************************************************
 *
 * @file 	bee_ota.h
 * @author 	tuha
 * @date 	3 July 2023
 * @brief	module for update ota
 *
 ***************************************************************************/

#ifndef BEE_OTA_H_
#define BEE_OTA_H_

#define VERSION 1.0

/**
 * @brief Start OTA (Over-The-Air) firmware update.
 * 
 * This function initiates the OTA firmware update process by configuring the
 * HTTP client with the provided URL and necessary settings. After downloading
 * the firmware update, the device is rebooted.
 * 
 * @param cUrl The URL to download the firmware update from.
 */
void start_ota(char *cUrl);

#endif