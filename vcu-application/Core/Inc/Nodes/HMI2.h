/*
 * HMI2.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI2_H_
#define INC_NODES_HMI2_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	uint8_t power;
	uint8_t started;
	uint32_t tick;
} hmi2_data_t;

typedef struct {
	struct {
		void (*State)(void);
	} r;
} hmi2_can_t;

typedef struct {
	hmi2_data_t d;
	hmi2_can_t can;
	void (*Init)(void);
	void (*Refresh)(void);
	void (*PowerOverCan)(uint8_t);
} hmi2_t;

/* Public functions implementation --------------------------------------------*/
void HMI2_Init(void);
void HMI2_Refresh(void);
void HMI2_PowerOverCan(uint8_t state);
void HMI2_CAN_RX_State(void);

/* ====================================== THREAD =================================== */
void StartHmi2PowerTask(void *argument);
#endif /* INC_NODES_HMI2_H_ */
