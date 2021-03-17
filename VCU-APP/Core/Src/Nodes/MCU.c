/*
 * MCU.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/MCU.h"

/* Public variables -----------------------------------------------------------*/
mcu_t MCU = {
		.d = { 0 },
		.can = {
				.r = {
						MCU_CAN_RX_State,
				},
		},
		MCU_Init,
		MCU_Refresh,
		MCU_SpeedToVolume
};

/* Private functions prototypes -----------------------------------------------*/
static void Reset(void);

/* Public functions implementation --------------------------------------------*/
void MCU_Init(void) {
	Reset();
}

void MCU_Refresh(void) {
	if ((_GetTickMS() - MCU.d.tick) > MCU_TIMEOUT)
		Reset();
}

uint16_t MCU_SpeedToVolume(void) {
  return MCU.d.speed * 100 / MCU_SPEED_KPH_MAX ;
}

/* ====================================== CAN RX =================================== */
void MCU_CAN_RX_State(can_rx_t *Rx) {
	// save state
	//	MCU.d.started = 1;
	MCU.d.tick = _GetTickMS();
	//	MCU.d.version = Rx->data.u16[0];
}

/* Private functions implementation --------------------------------------------*/
static void Reset(void) {
	MCU.d.speed = 0;
	//	MCU.d.started = 0;
	//	MCU.d.tick = 0;
	//	MCU.d.version = 0;
}
