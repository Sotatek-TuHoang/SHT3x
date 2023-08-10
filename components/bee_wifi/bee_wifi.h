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

void wifi_init_func(void);

void wifi_prov(void);

void prov_timeout_task(void* pvParameters);
void prov_fail_task(void* pvParameters);

#endif /* BEE_WIFI_H_ */

