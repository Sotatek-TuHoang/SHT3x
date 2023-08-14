/*****************************************************************************
 *
 * @file 	bee_wifi.h
 * @author 	tuha
 * @date 	5 July 2023
 * @brief	module for connect wifi
 *
 ***************************************************************************/

/****************************************************************************/


#ifndef BEE_WIFI_H_
#define BEE_WIFI_H_

/**
 * @brief   Initialize Wi-Fi functionality, event handlers, and provisioning.
 *
 * This function initializes the TCP/IP stack, creates the event loop, sets up event handlers for Wi-Fi, IP, and provisioning events,
 * initializes the default Wi-Fi station interface, and configures the Wi-Fi provisioning manager. 
 * If the device is already provisioned, it starts the Wi-Fi station; otherwise, it stops provisioning.
 * The function blocks until the device is connected to Wi-Fi.
 *
 * @param   None
 * @return  None
 */
void wifi_func_init(void);

/**
 * @brief   Start the Wi-Fi provisioning process.
 *
 * This function initiates the Wi-Fi provisioning process if it has not already started. It sets up the necessary parameters
 * for provisioning, including security settings and service name. If the provisioning process starts successfully, it registers
 * an endpoint for handling custom provisioning data and creates a task to monitor the provisioning timeout.
 *
 * @note    This function will have no effect if provisioning is already in progress.
 *
 * @param   None
 * @return  None
 */
void wifi_prov();

/**
 * @brief   Task for handling provisioning timeout.
 *
 * This task monitors the provisioning process and waits for a specified timeout duration before stopping the provisioning
 * process, restoring the previous Wi-Fi connection, and then deleting itself.
 *
 * @param   pvParameters Pointer to task parameters (unused in this context).
 * @return  None
 */
void prov_timeout_task(void* pvParameters);

/**
 * @brief   Task for handling provisioning failure.
 *
 * This task is responsible for resetting the provisioning state on failure and then initiating a timeout period
 * before stopping the provisioning process, attempting to reconnect to the previous Wi-Fi connection, and finally deleting itself.
 *
 * @param   pvParameters Pointer to task parameters (unused in this context).
 * @return  None
 */
void prov_fail_task(void* pvParameters);

#endif /* BEE_WIFI_H_ */

