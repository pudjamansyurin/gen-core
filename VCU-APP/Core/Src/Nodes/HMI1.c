/*
 * HMI1.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/HMI1.h"

/* Public variables -----------------------------------------------------------*/
hmi1_t HMI1 = {
		.d = { 0 },
		.can = {
				.r = {
						HMI1_CAN_RX_State,
				},
		},
		HMI1_Init,
		HMI1_Refresh,
		GATE_Hmi1Power
};

/* Public functions implementation --------------------------------------------*/
void HMI1_Init(void) {
	// reset HMI1 data
	HMI1.d.started = 0;
	HMI1.d.tick = 0;
	HMI1.d.version = 0;
	HMI1.d.state.mirroring = 0;
	HMI1.d.state.warning = 0;
	HMI1.d.state.overheat = 0;
	HMI1.d.state.finger = 0;
  HMI1.d.state.unremote = 1;
	HMI1.d.state.daylight = 0;
}

void HMI1_Refresh(void) {
	if ((_GetTickMS() - HMI1.d.tick) > 10000) {
		HMI1.d.started = 0;
		HMI1.d.tick = 0;
		HMI1.d.version = 0;
	}
}

/* ====================================== CAN RX =================================== */
void HMI1_CAN_RX_State(can_rx_t *Rx) {
	// save state
	HMI1.d.started = 1;
	HMI1.d.tick = _GetTickMS();
	HMI1.d.version = Rx->data.u16[0];
}
