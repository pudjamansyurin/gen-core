/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "VCU.h"
#include "BMS.h"
#include "HMI1.h"
#include "_canbus.h"
#include "_utils.h"

/* External variables ---------------------------------------------------------*/
extern canbus_t CB;
extern bms_t BMS;
extern hmi1_t HMI1;

/* Public variables -----------------------------------------------------------*/
vcu_t VCU = {
		.d = { 0 },
		.can = {
				.t = {
						VCU_CAN_TX_Switch,
						VCU_CAN_TX_Datetime,
						VCU_CAN_TX_SelectSet,
						VCU_CAN_TX_TripMode
				}
		},
		VCU_Init,
		VCU_SetEvent,
		VCU_ReadEvent,
		VCU_CheckMainPower,
};

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void) {
	// reset VCU data
	VCU.d.knob = 0;
	VCU.d.independent = 1;
	VCU.d.interval = RPT_INTERVAL_SIMPLE;
	VCU.d.volume = 0;
	VCU.d.bat_voltage = 0;
	VCU.d.signal_percent = 0;
	VCU.d.speed = 0;
	VCU.d.odometer = 0;
	VCU.d.events = 0;
	VCU.d.tick.keyless = 0;
	//  VCU.d.tick.finger = 0;
	VCU.d.seq_id.report = 0;
	VCU.d.seq_id.response = 0;
}

void VCU_SetEvent(uint64_t event_id, uint8_t value) {
	if (value & 1) {
		BV(VCU.d.events, _BitPosition(event_id));
	} else {
		BC(VCU.d.events, _BitPosition(event_id));
	}
}

uint8_t VCU_ReadEvent(uint64_t event_id) {
	return (VCU.d.events & event_id) == event_id;
}

void VCU_CheckMainPower(void) {
	VCU.d.independent = !HAL_GPIO_ReadPin(EXT_BMS_IRQ_GPIO_Port, EXT_BMS_IRQ_Pin);
	VCU.d.interval = VCU.d.independent ? RPT_INTERVAL_INDEPENDENT : RPT_INTERVAL_SIMPLE;
	VCU.SetEvent(EV_VCU_INDEPENDENT, VCU.d.independent);
}

/* ====================================== CAN TX =================================== */
uint8_t VCU_CAN_TX_Switch(sw_t *sw) {
	sein_state_t sein = HBAR_SeinController(sw);

	// set message
	CB.tx.data.u8[0] = sw->list[SW_K_ABS].state;
	CB.tx.data.u8[0] |= _L(HMI1.d.status.mirroring, 1);
	CB.tx.data.u8[0] |= _L(sw->list[SW_K_LAMP].state, 2);
	CB.tx.data.u8[0] |= _L(HMI1.d.status.warning, 3);
	CB.tx.data.u8[0] |= _L(HMI1.d.status.overheat, 4);
	CB.tx.data.u8[0] |= _L(HMI1.d.status.finger, 5);
	CB.tx.data.u8[0] |= _L(HMI1.d.status.keyless, 6);
	CB.tx.data.u8[0] |= _L(HMI1.d.status.daylight, 7);

	// sein value
	CB.tx.data.u8[1] = sein.left;
	CB.tx.data.u8[1] |= _L(sein.right, 1);
	CB.tx.data.u8[2] = VCU.d.signal_percent;
	CB.tx.data.u8[3] = BMS.d.soc;

	// odometer
	CB.tx.data.u32[1] = VCU.d.odometer;

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_SWITCH, 8);
	// send message
	return CANBUS_Write(&(CB.tx));
}

uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp) {
	// set message
	CB.tx.data.u8[0] = timestamp->time.Seconds;
	CB.tx.data.u8[1] = timestamp->time.Minutes;
	CB.tx.data.u8[2] = timestamp->time.Hours;
	CB.tx.data.u8[3] = timestamp->date.Date;
	CB.tx.data.u8[4] = timestamp->date.Month;
	CB.tx.data.u8[5] = timestamp->date.Year;
	CB.tx.data.u8[6] = timestamp->date.WeekDay;
	// HMI2 shutdown request
	CB.tx.data.u8[7] = !VCU.d.knob;

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_DATETIME, 8);
	// send message
	return CANBUS_Write(&(CB.tx));
}

uint8_t VCU_CAN_TX_SelectSet(sw_runner_t *runner) {
	// set message
	CB.tx.data.u8[0] = runner->mode.sub.val[SW_M_DRIVE];
	CB.tx.data.u8[0] |= _L(runner->mode.sub.val[SW_M_TRIP], 2);
	CB.tx.data.u8[0] |= _L(runner->mode.sub.val[SW_M_REPORT], 3);
	CB.tx.data.u8[0] |= _L(runner->mode.val, 4);

	// Send Show/Hide flag
	CB.tx.data.u8[0] |= _L(HBAR_ModeController(runner), 6);

	CB.tx.data.u8[1] = runner->mode.sub.report[SW_M_REPORT_RANGE];
	CB.tx.data.u8[2] = runner->mode.sub.report[SW_M_REPORT_EFFICIENCY];

	CB.tx.data.u8[3] = VCU.d.speed;

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_SELECT_SET, 4);
	// send message
	return CANBUS_Write(&(CB.tx));
}

uint8_t VCU_CAN_TX_TripMode(uint32_t *trip) {
	// set message
	CB.tx.data.u32[0] = trip[SW_M_TRIP_A];
	CB.tx.data.u32[1] = trip[SW_M_TRIP_B];

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_TRIP_MODE, 8);
	// send message
	return CANBUS_Write(&(CB.tx));
}
