/*
 * HMI1.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/HMI1.h"

/* Public variables
 * -----------------------------------------------------------*/
hmi1_t HMI1 = {
		.d = {0},
		.r ={HMI1_RX_State},
		.Init = HMI1_Init,
		.Refresh = HMI1_Refresh,
		.Power = GATE_Hmi1Power
};

/* Private functions prototypes
 * -----------------------------------------------*/
static void Reset(void);

/* Public functions implementation
 * --------------------------------------------*/
void HMI1_Init(void) {
	Reset();
}

void HMI1_Refresh(void) {
	HMI1.d.active = HMI1.d.tick && (_GetTickMS() - HMI1.d.tick) < HMI1_TIMEOUT_MS;

	if (!HMI1.d.active)
		Reset();
}

/* ====================================== CAN RX
 * =================================== */
void HMI1_RX_State(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	HMI1.d.version = d->u16[0];

	HMI1.d.tick = _GetTickMS();
}

/* Private functions implementation
 * --------------------------------------------*/
static void Reset(void) {
	HMI1.d.active = 0;
	HMI1.d.tick = 0;
	HMI1.d.version = 0;
}
