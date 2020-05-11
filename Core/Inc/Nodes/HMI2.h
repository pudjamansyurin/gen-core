/*
 * HMI2.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI2_H_
#define INC_NODES_HMI2_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	uint8_t started;
	uint32_t tick;
} hmi2_data_t;

typedef struct {
	hmi2_data_t d;
	void (*Init)(void);
	void (*Refresh)(void);
} hmi2_t;

/* Public functions implementation --------------------------------------------*/
void HMI2_Init(void);
void HMI2_Refresh(void);

#endif /* INC_NODES_HMI2_H_ */
