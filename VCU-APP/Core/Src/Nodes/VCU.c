/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/VCU.h"
#include "Drivers/_bat.h"
#include "Drivers/_simcom.h"
#include "Libs/_remote.h"
#include "Libs/_finger.h"
#include "Nodes/NODE.h"
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"

/* Public variables
 * -----------------------------------------------------------*/
vcu_t VCU = {
		.d = {0},
};

/* External variables -----------------------------------------*/
extern osMessageQueueId_t ReportQueueHandle;

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void) {
	memset(&(VCU.d), 0, sizeof(vcu_data_t));
	VCU.d.state = VEHICLE_BACKUP;
}

void VCU_Refresh(void) {
	BAT_ScanValue();

	VCU.d.uptime++;
	VCU.d.buffered = osMessageQueueGetCount(ReportQueueHandle);
	VCU.d.error = EVT_Get(EVG_BIKE_FALLEN);
}

/* ====================================== CAN TX
 * =================================== */
uint8_t VCU_TX_Heartbeat(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	d->u16[0] = VCU_VERSION;

	return CANBUS_Write(&Tx, CAND_VCU, 2, 0);
}

uint8_t VCU_TX_SwitchControl(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	d->u8[0] = HBAR.d.pin[HBAR_K_ABS];
	d->u8[0] |= HBAR.d.pin[HBAR_K_LAMP] << 2;
	d->u8[0] |= NODE.d.error << 3;
	d->u8[0] |= NODE.d.overheat << 4;
	d->u8[0] |= !FGR.d.id << 5;
	d->u8[0] |= !RMT.d.nearby << 6;
	d->u8[0] |= RTC_Daylight() << 7;

	// sein value
	HBAR_RefreshSein();
	d->u8[1] = HBAR.ctl.sein.left;
	d->u8[1] |= HBAR.ctl.sein.right << 1;
	//	d->u8[1] |= HBAR.d.pin[HBAR_K_REVERSE] << 2;
	d->u8[1] |= MCU_Reversed() << 2;
	d->u8[1] |= BMS.d.run << 3;
	d->u8[1] |= MCU.d.run << 4;
	d->u8[1] |= (FGR.d.registering & 0x03) << 5;

	// mode
	d->u8[2] = HBAR.d.mode[HBAR_M_DRIVE];
	//	d->u8[2] = MCU.d.drive_mode;
	d->u8[2] |= HBAR.d.mode[HBAR_M_TRIP] << 2;
	d->u8[2] |= HBAR.d.mode[HBAR_M_REPORT] << 4;
	d->u8[2] |= HBAR.d.m << 5;
	d->u8[2] |= (HBAR.ctl.session > 0) << 7;

	// others
	d->u8[3] = MCU_RpmToSpeed(MCU.d.rpm);
	d->u8[4] = (uint8_t) MCU.d.dcbus.current;
	d->u8[5] = BMS.d.soc;
	d->u8[6] = SIM.d.signal;
	d->u8[7] = (int8_t) VCU.d.state;

	// send message
	return CANBUS_Write(&Tx, CAND_VCU_SWITCH_CTL, 8, 0);
}

uint8_t VCU_TX_Datetime(datetime_t dt) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	uint8_t hmi2shutdown = VCU.d.state < VEHICLE_STANDBY;

	d->u8[0] = dt.Seconds;
	d->u8[1] = dt.Minutes;
	d->u8[2] = dt.Hours;
	d->u8[3] = dt.Date;
	d->u8[4] = dt.Month;
	d->u8[5] = dt.Year;
	d->u8[6] = dt.WeekDay;
	d->u8[7] = hmi2shutdown;

	return CANBUS_Write(&Tx, CAND_VCU_DATETIME, 8, 0);
}

uint8_t VCU_TX_ModeData(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	d->u16[0] = HBAR.d.trip[HBAR_M_TRIP_A];
	d->u16[1] = HBAR.d.trip[HBAR_M_TRIP_B];
	d->u16[2] = HBAR.d.trip[HBAR_M_TRIP_ODO];
	d->u8[6] = HBAR.d.report[HBAR_M_REPORT_RANGE];
	d->u8[7] = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

	return CANBUS_Write(&Tx, CAND_VCU_MODE_DATA, 8, 0);
}
/* Private functions implementation -------------------------------------------*/
