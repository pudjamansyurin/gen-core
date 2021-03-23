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
		HMI1_Init,
		HMI1_Refresh,
		GATE_Hmi1Power
};

/* Private functions prototypes
 * -----------------------------------------------*/
static void Reset(void);

/* Public functions implementation
 * --------------------------------------------*/
void HMI1_Init(void) {
	HMI1.d.state.mirroring = 0;
	HMI1.d.state.warning = 0;
	HMI1.d.state.overheat = 0;
//	HMI1.d.state.unfinger = 1;
//	HMI1.d.state.unremote = 1;
//	HMI1.d.state.daylight = 0;

	Reset();
}

void HMI1_Refresh(void) {
	HMI1.d.run = HMI1.d.tick && (_GetTickMS() - HMI1.d.tick) < HMI1_TIMEOUT;

	if (!HMI1.d.run)
		Reset();
}

/* ====================================== CAN RX
 * =================================== */
void HMI1_RX_State(can_rx_t *Rx) {
	HMI1.d.version = Rx->data.u16[0];

	HMI1.d.tick = _GetTickMS();
}

/* Private functions implementation
 * --------------------------------------------*/
static void Reset(void) {
	HMI1.d.run = 0;
	HMI1.d.tick = 0;
	HMI1.d.version = 0;
}
