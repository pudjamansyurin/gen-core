/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_can.h"
#include "_reporter.h"
#include "_handlebar.h"
#include "VCU.h"
#include "BMS.h"
#include "HMI1.h"
#include "HMI2.h"

/* External variables ----------------------------------------------------------*/
extern canbus_t CB;
extern vcu_t VCU;
extern bms_t BMS;
extern hmi1_t HMI1;
extern hmi2_t HMI2;

/* Public functions implementation --------------------------------------------*/
uint8_t CANT_VCU_Switch(sw_t *sw) {
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
	CB.tx.data.u8[1] |= _L(!VCU.d.knob, 2);
	CB.tx.data.u8[2] = VCU.d.signal_percent;
	CB.tx.data.u8[3] = BMS.d.soc;

	// odometer
	CB.tx.data.u32[1] = VCU.d.odometer;

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_SWITCH, 8);
	// send message
	return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_VCU_RTC(timestamp_t *timestamp) {
	// set message
	CB.tx.data.u8[0] = timestamp->time.Seconds;
	CB.tx.data.u8[1] = timestamp->time.Minutes;
	CB.tx.data.u8[2] = timestamp->time.Hours;
	CB.tx.data.u8[3] = timestamp->date.Date;
	CB.tx.data.u8[4] = timestamp->date.Month;
	CB.tx.data.u8[5] = timestamp->date.Year;
	CB.tx.data.u8[6] = timestamp->date.WeekDay;

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_RTC, 7);
	// send message
	return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_VCU_SelectSet(sw_runner_t *runner) {
	uint8_t hide = HBAR_ModeController(runner);

	// set message
	CB.tx.data.u8[0] = runner->mode.sub.val[SW_M_DRIVE];
	CB.tx.data.u8[0] |= _L(runner->mode.sub.val[SW_M_TRIP], 2);
	CB.tx.data.u8[0] |= _L(runner->mode.sub.val[SW_M_REPORT], 3);
	CB.tx.data.u8[0] |= _L(runner->mode.val, 4);

	// Send Show/Hide flag
	CB.tx.data.u8[0] |= _L(hide, 6);

	CB.tx.data.u8[1] = runner->mode.sub.report[SW_M_REPORT_RANGE];
	CB.tx.data.u8[2] = runner->mode.sub.report[SW_M_REPORT_EFFICIENCY];

	CB.tx.data.u8[3] = VCU.d.speed;

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_SELECT_SET, 4);
	// send message
	return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_VCU_TripMode(uint32_t *trip) {
	// set message
	CB.tx.data.u32[0] = trip[SW_M_TRIP_A];
	CB.tx.data.u32[1] = trip[SW_M_TRIP_B];

	// set default header
	CANBUS_Header(&(CB.tx.header), CAND_VCU_TRIP_MODE, 8);
	// send message
	return CANBUS_Write(&(CB.tx));
}

