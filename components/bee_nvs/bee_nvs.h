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

#endif /* BEE_NVS_H */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/