/*
 * HMI2.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/*
 * HMI1.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/HMI2.h"
#include "Nodes/HMI1.h"

/* External variables
 * ---------------------------------------------------------*/
extern osThreadId_t Hmi2PowerTaskHandle;

/* Public variables
 * -----------------------------------------------------------*/
hmi2_t HMI2 = {
		.d = {0},
		.r = {
				HMI2_RX_State
		},
		.Init = HMI2_Init,
		.Refresh = HMI2_Refresh,
		.PowerByCAN = HMI2_PowerByCAN,
		.PowerOn = HMI2_PowerOn,
		.PowerOff = HMI2_PowerOff
};

/* Public functions implementation
 * --------------------------------------------*/
void HMI2_Init(void) {
	memset(&(HMI2.d), 0, sizeof(hmi2_data_t));
}

void HMI2_Refresh(void) {
	HMI2.d.run = HMI2.d.tick && (_GetTickMS() - HMI2.d.tick) < HMI2_TIMEOUT_MS;

	if (!HMI2.d.run) HMI2.d.mirroring = 0;
}

void HMI2_PowerByCAN(uint8_t state) {
	if (HMI2.d.powerRequest != state) {
		HMI2.d.powerRequest = state;
		osThreadFlagsSet(Hmi2PowerTaskHandle, FLAG_HMI2POWER_CHANGED);
	}
}

void HMI2_PowerOn(void) {
	TickType_t tick;

	GATE_Hmi2Reset();

	// wait until turned ON by CAN
	tick = _GetTickMS();
	while (!HMI2.d.run && _GetTickMS() - tick < HMI2_POWER_ON_MS) {};
}

void HMI2_PowerOff(void) {
	TickType_t tick;

	// wait until turned OFF by CAN
	tick = _GetTickMS();
	while (HMI2.d.run && _GetTickMS() - tick < HMI2_POWER_OFF_MS) {};

	GATE_Hmi2Stop();
}

/* ====================================== CAN RX
 * =================================== */
void HMI2_RX_State(can_rx_t *Rx) {
	UNION64 *d = &(Rx->data);

	HMI2.d.mirroring = (d->u8[0] >> 0) & 0x01;

	// save state
	HMI2.d.tick = _GetTickMS();
}
