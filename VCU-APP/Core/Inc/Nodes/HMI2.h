/*
 * HMI2.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI2_H_
#define INC_NODES_HMI2_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Libs/_utils.h"

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	uint8_t power;
	uint8_t started;
	uint32_t tick;
} hmi2_data_t;

typedef struct {
	struct {
		void (*State)(can_rx_t*);
	} r;
} hmi2_can_t;

typedef struct {
	hmi2_data_t d;
	hmi2_can_t can;
	void (*Init)(void);
	void (*Refresh)(void);
	uint8_t (*PowerOverCan)(uint8_t);
	void (*PowerOn)(void);
	void (*PowerOff)(void);
} hmi2_t;

/* Exported variables ---------------------------------------------------------*/
extern hmi2_t HMI2;

/* Public functions implementation --------------------------------------------*/
void HMI2_Init(void);
void HMI2_Refresh(void);
uint8_t HMI2_PowerOverCan(uint8_t state);
void HMI2_CAN_RX_State(can_rx_t *Rx);
void HMI2_PowerOn(void);
void HMI2_PowerOff(void);

#endif /* INC_NODES_HMI2_H_ */
