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
						MCU_CAN_RX_CurrentDC,
						MCU_CAN_RX_VoltageDC,
						MCU_CAN_RX_TorqueSpeed,
						MCU_CAN_RX_FaultCode,
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
	return MCU.d.speed * 100 / MCU_SPEED_MAX ;
}

/* ====================================== CAN RX =================================== */
void MCU_CAN_RX_CurrentDC(can_rx_t *Rx) {
	MCU.d.dcbus.current = Rx->data.u16[3] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_VoltageDC(can_rx_t *Rx) {
	MCU.d.dcbus.voltage = Rx->data.u16[1] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_TorqueSpeed(can_rx_t *Rx) {
	MCU.d.temperature = Rx->data.u16[0] * 0.1;
	MCU.d.rpm = Rx->data.u16[1];
	MCU.d.torque.commanded = Rx->data.u16[2] * 0.1;
	MCU.d.torque.feedback = Rx->data.u16[3] * 0.1;

	// TODO: convert rpm to speed;
	MCU.d.speed = MCU.d.rpm * MCU_SPEED_MAX / MCU_RPM_MAX;

	MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_FaultCode(can_rx_t *Rx) {
	MCU.d.fault.post = Rx->data.u32[0];
	MCU.d.fault.run = Rx->data.u32[1];

	MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_State(can_rx_t *Rx) {
	MCU.d.drive_mode = Rx->data.u8[4] & 0x03;
	MCU.d.inv.discharge = (Rx->data.u8[4] >> 5) & 0x07;
	MCU.d.inv.can_mode = Rx->data.u8[5];
	MCU.d.inv.enabled = Rx->data.u8[6] & 0x01;
	MCU.d.inv.lockout = (Rx->data.u8[6] >> 7) & 0x01;
	MCU.d.reverse = Rx->data.u8[7];

	MCU.d.tick = _GetTickMS();
}


/* Private functions implementation --------------------------------------------*/
static void Reset(void) {
	MCU.d.run = 0;
	MCU.d.tick = 0;

	MCU.d.rpm = 0;
	MCU.d.speed = 0;
	MCU.d.reverse = 0;
	MCU.d.temperature = 0;
	MCU.d.drive_mode = DRIVE_MODE_ECONOMIC;

	MCU.d.torque.commanded = 0;
	MCU.d.torque.feedback = 0;

	MCU.d.fault.post = 0;
	MCU.d.fault.run = 0;

	MCU.d.dcbus.current = 0;
	MCU.d.dcbus.voltage = 0;

	MCU.d.inv.can_mode = 0;
	MCU.d.inv.enabled = 0;
	MCU.d.inv.lockout = 0;
	MCU.d.inv.discharge = INV_DISCHARGE_DISABLED;
}
