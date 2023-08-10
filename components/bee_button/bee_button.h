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

void button_init();
void wifi_prov_button_isr(void* arg);

#endif
