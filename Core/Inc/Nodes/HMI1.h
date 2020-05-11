/*
 * HMI1.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI1_H_
#define INC_NODES_HMI1_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Exported enum -------------------------------------------------------------*/
typedef enum {
	HMI1_DEV_LEFT = 0,
	HMI1_DEV_RIGHT = 1,
	HMI1_DEV_MAX = 1
} HMI1_DEVICE;

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	uint8_t started;
	struct {
		//	uint8_t abs;
		uint8_t mirroring;
		//  uint8_t lamp;
		uint8_t warning;
		uint8_t overheat;
		uint8_t finger;
		uint8_t keyless;
		uint8_t daylight;
	//	uint8_t sein_left;
	//	uint8_t sein_right;
	} status;
	struct {
		uint8_t started;
		uint32_t tick;
	} device[2];
} hmi1_data_t;

typedef struct {
	struct {
		void (*LeftState)(void);
		void (*RightState)(void);
	} r;
} hmi1_can_t;

typedef struct {
	hmi1_data_t d;
	hmi1_can_t can;
	void (*Init)(void);
	void (*RefreshIndex)(void);
} hmi1_t;

/* Public functions implementation --------------------------------------------*/
void HMI1_Init(void);
void HMI1_RefreshIndex(void);
void HMI1_CAN_RX_LeftState(void);
void HMI1_CAN_RX_RightState(void);

#endif /* INC_NODES_HMI1_H_ */
