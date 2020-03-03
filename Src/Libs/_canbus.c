/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_canbus.h"
#include "_rtc.h"

extern const TickType_t tick5000ms;
extern const TickType_t tick500ms;
extern const TickType_t tick250ms;
extern vcu_t DB_VCU;
extern hmi1_t DB_HMI1;
extern hmi2_t DB_HMI2;
extern CAN_Rx RxCan;
CAN_Tx TxCan;

// ==================================== VCU =========================================
#if (CAN_NODE & CAN_NODE_VCU)
uint8_t CANBUS_VCU_Switch(void) {
	static TickType_t tick, tickSein;
	static uint8_t Sein_Left_Internal = 0, Sein_Right_Internal = 0;
	static status_t DB_HMI_Status_Internal;

	// indicator manipulator
	if ((osKernelSysTick() - tick) >= tick500ms) {
		// finger
		if (DB_HMI1.status.finger) {
			tick = osKernelSysTick();
			DB_HMI_Status_Internal.finger = !DB_HMI_Status_Internal.finger;
		}
		// keyless
		if (DB_HMI1.status.keyless) {
			tick = osKernelSysTick();
			DB_HMI_Status_Internal.keyless = !DB_HMI_Status_Internal.keyless;
		}
		// temperature
		if (DB_HMI1.status.temperature) {
			tick = osKernelSysTick();
			DB_HMI_Status_Internal.temperature = !DB_HMI_Status_Internal.temperature;
		}
	}

	// sein manipulator
	if ((osKernelSysTick() - tickSein) >= tick500ms) {
		if (DB_VCU.sw.list[IDX_KEY_SEIN_LEFT].state && DB_VCU.sw.list[IDX_KEY_SEIN_RIGHT].state) {
			// hazard
			tickSein = osKernelSysTick();
			Sein_Left_Internal = !Sein_Left_Internal;
			Sein_Right_Internal = Sein_Left_Internal;
		} else if (DB_VCU.sw.list[IDX_KEY_SEIN_LEFT].state) {
			// left sein
			tickSein = osKernelSysTick();
			Sein_Left_Internal = !Sein_Left_Internal;
			Sein_Right_Internal = 0;
		} else if (DB_VCU.sw.list[IDX_KEY_SEIN_RIGHT].state) {
			// right sein
			tickSein = osKernelSysTick();
			Sein_Left_Internal = 0;
			Sein_Right_Internal = !Sein_Right_Internal;
		} else {
			Sein_Left_Internal = 0;
			Sein_Right_Internal = Sein_Left_Internal;
		}
	}

	// set message
	TxCan.TxData[0] = DB_VCU.sw.list[IDX_KEY_ABS].state;
	TxCan.TxData[0] |= DB_VCU.sw.list[IDX_KEY_MIRRORING].state << 1;
	TxCan.TxData[0] |= 1 << 2;
	TxCan.TxData[0] |= 1 << 3;
	TxCan.TxData[0] |= DB_HMI_Status_Internal.temperature << 4;
	TxCan.TxData[0] |= DB_HMI_Status_Internal.finger << 5;
	TxCan.TxData[0] |= DB_HMI_Status_Internal.keyless << 6;
	// check daylight (for auto brightness of HMI)
	RTC_Read_RAW(&DB_VCU.timestamp);
	TxCan.TxData[0] |= (DB_VCU.timestamp.time.Hours >= 5 && DB_VCU.timestamp.time.Hours <= 16) << 7;

	// sein value
	TxCan.TxData[1] = Sein_Left_Internal << 0;
	TxCan.TxData[1] |= Sein_Right_Internal << 1;

	// HMI-2 Shutdown Request
	TxCan.TxData[1] |= DB_HMI2.shutdown << 2;

	// signal strength
	TxCan.TxData[2] = DB_VCU.signal;

	// odometer
	TxCan.TxData[4] = (DB_VCU.odometer >> 0) & 0xFF;
	TxCan.TxData[5] = (DB_VCU.odometer >> 8) & 0xFF;
	TxCan.TxData[6] = (DB_VCU.odometer >> 16) & 0xFF;
	TxCan.TxData[7] = (DB_VCU.odometer >> 24) & 0xFF;

	// dummy algorithm
	DB_VCU.odometer = (DB_VCU.odometer >= VCU_ODOMETER_MAX ? 0 : (DB_VCU.odometer + 1));

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_SWITCH, 8);
	// send message
	return CAN_Write(&TxCan);
}

uint8_t CANBUS_VCU_RTC(void) {
	// set message
	RTC_Read_RAW(&DB_VCU.timestamp);
	TxCan.TxData[0] = DB_VCU.timestamp.time.Seconds;
	TxCan.TxData[1] = DB_VCU.timestamp.time.Minutes;
	TxCan.TxData[2] = DB_VCU.timestamp.time.Hours;
	TxCan.TxData[3] = DB_VCU.timestamp.date.Date;
	TxCan.TxData[4] = DB_VCU.timestamp.date.Month;
	TxCan.TxData[5] = DB_VCU.timestamp.date.Year;
	TxCan.TxData[6] = DB_VCU.timestamp.date.WeekDay;

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_RTC, 7);
	// send message
	return CAN_Write(&TxCan);
}

uint8_t CANBUS_VCU_Select_Set(void) {
	static TickType_t tick, tickPeriod;
	static uint8_t Mode_Hide_Internal = 0;
	static int8_t Mode_Name_Internal = -1;
	static int8_t Mode_Value_Internal = -1;

	// Mode Show/Hide Manipulator
	if (DB_VCU.sw.runner.listening) {
		// if mode same
		if (Mode_Name_Internal != DB_VCU.sw.runner.mode.val) {
			Mode_Name_Internal = DB_VCU.sw.runner.mode.val;
			// reset period tick
			tickPeriod = osKernelSysTick();
		} else if (Mode_Value_Internal != DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val]) {
			Mode_Value_Internal = DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val];
			// reset period tick
			tickPeriod = osKernelSysTick();
		}

		if ((osKernelSysTick() - tickPeriod) >= tick5000ms ||
				(DB_VCU.sw.runner.mode.sub.val[SWITCH_MODE_DRIVE] == SWITCH_MODE_DRIVE_R)) {
			// stop listening
			DB_VCU.sw.runner.listening = 0;
			Mode_Hide_Internal = 0;
			Mode_Name_Internal = -1;
			Mode_Value_Internal = -1;
		} else {
			// blink
			if ((osKernelSysTick() - tick) >= tick250ms) {
				tick = osKernelSysTick();
				Mode_Hide_Internal = !Mode_Hide_Internal;
			}
		}
	} else {
		Mode_Hide_Internal = 0;
	}

	// set message
	TxCan.TxData[0] = DB_VCU.sw.runner.mode.sub.val[SWITCH_MODE_DRIVE];
	TxCan.TxData[0] |= DB_VCU.sw.runner.mode.sub.val[SWITCH_MODE_TRIP] << 2;
	TxCan.TxData[0] |= DB_VCU.sw.runner.mode.sub.val[SWITCH_MODE_REPORT] << 3;
	TxCan.TxData[0] |= DB_VCU.sw.runner.mode.val << 4;

	// Send Show/Hide flag
	TxCan.TxData[0] |= Mode_Hide_Internal << 6;

	TxCan.TxData[1] = DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_RANGE];
	TxCan.TxData[2] = DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_AVERAGE];

	// dummy algorithm
	if (!DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_RANGE]) {
		DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_RANGE] = 255;
	} else {
		DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_RANGE]--;
	}

	if (DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_AVERAGE] >= 255) {
		DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_AVERAGE] = 0;
	} else {
		DB_VCU.sw.runner.mode.sub.report[SWITCH_MODE_REPORT_AVERAGE]++;
	}

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_SELECT_SET, 3);
	// send message
	return CAN_Write(&TxCan);
}

uint8_t CANBUS_VCU_Trip_Mode(void) {
	// set message
	TxCan.TxData[0] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A] >> 0) & 0xFF;
	TxCan.TxData[1] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A] >> 8) & 0xFF;
	TxCan.TxData[2] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A] >> 16) & 0xFF;
	TxCan.TxData[3] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A] >> 24) & 0xFF;
	TxCan.TxData[4] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B] >> 0) & 0xFF;
	TxCan.TxData[5] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B] >> 8) & 0xFF;
	TxCan.TxData[6] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B] >> 16) & 0xFF;
	TxCan.TxData[7] = (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B] >> 24) & 0xFF;

	// dummy algorithm
	if (DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val] == SWITCH_MODE_TRIP_A) {
		if (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A] >= VCU_ODOMETER_MAX) {
			DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A] = 0;
		} else {
			DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_A]++;
		}
	} else {
		if (DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B] >= VCU_ODOMETER_MAX) {
			DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B] = 0;
		} else {
			DB_VCU.sw.runner.mode.sub.trip[SWITCH_MODE_TRIP_B]++;
		}
	}
	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_TRIP_MODE, 8);
	// send message
	return CAN_Write(&TxCan);
}

/* ------------------------------------ READER ------------------------------------- */
void CANBUS_MCU_Dummy_Read(void) {
	uint32_t DB_MCU_RPM;

	// read message
	DB_MCU_RPM = (RxCan.RxData[3] << 24 | RxCan.RxData[2] << 16 | RxCan.RxData[1] << 8 | RxCan.RxData[0]);
	// convert RPM to Speed
	DB_VCU.speed = DB_MCU_RPM * MCU_SPEED_MAX / MCU_RPM_MAX;
}
#endif

