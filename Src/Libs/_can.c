/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_can.h"
#include "_rtc.h"

extern const TickType_t tick5000ms;
extern const TickType_t tick500ms;
extern const TickType_t tick250ms;
extern vcu_t DB_VCU;
extern hmi1_t DB_HMI1;
extern hmi2_t DB_HMI2;
extern CANBUS_Rx RxCan;
CANBUS_Tx TxCan;

// ==================================== VCU =========================================
#if (CAN_NODE & CAN_NODE_VCU)
uint8_t CAN_VCU_Switch(void) {
	static TickType_t tick, tickSein;
	static uint8_t iSein_Left = 0, iSein_Right = 0;
	static status_t iDB_HMI1_Status;

	// indicator manipulator
	if ((osKernelSysTick() - tick) >= tick500ms) {
		// finger
		if (DB_HMI1.status.finger) {
			tick = osKernelSysTick();
			iDB_HMI1_Status.finger = !iDB_HMI1_Status.finger;
		}
		// keyless
		if (DB_HMI1.status.keyless) {
			tick = osKernelSysTick();
			iDB_HMI1_Status.keyless = !iDB_HMI1_Status.keyless;
		}
		// temperature
		if (DB_HMI1.status.temperature) {
			tick = osKernelSysTick();
			iDB_HMI1_Status.temperature = !iDB_HMI1_Status.temperature;
		}
	}

	// sein manipulator
	if ((osKernelSysTick() - tickSein) >= tick500ms) {
		if (DB_VCU.sw.list[SW_K_SEIN_LEFT].state && DB_VCU.sw.list[SW_K_SEIN_RIGHT].state) {
			// hazard
			tickSein = osKernelSysTick();
			iSein_Left = !iSein_Left;
			iSein_Right = iSein_Left;
		} else if (DB_VCU.sw.list[SW_K_SEIN_LEFT].state) {
			// left sein
			tickSein = osKernelSysTick();
			iSein_Left = !iSein_Left;
			iSein_Right = 0;
		} else if (DB_VCU.sw.list[SW_K_SEIN_RIGHT].state) {
			// right sein
			tickSein = osKernelSysTick();
			iSein_Left = 0;
			iSein_Right = !iSein_Right;
		} else {
			iSein_Left = 0;
			iSein_Right = iSein_Left;
		}
	}

	// set message
	TxCan.TxData[0] = DB_VCU.sw.list[SW_K_ABS].state;
	TxCan.TxData[0] |= DB_VCU.sw.list[SW_K_MIRRORING].state << 1;
	TxCan.TxData[0] |= 1 << 2;
	TxCan.TxData[0] |= 1 << 3;
	TxCan.TxData[0] |= iDB_HMI1_Status.temperature << 4;
	TxCan.TxData[0] |= iDB_HMI1_Status.finger << 5;
	TxCan.TxData[0] |= iDB_HMI1_Status.keyless << 6;
	// check daylight (for auto brightness of HMI)
	RTC_Read_RAW(&DB_VCU.timestamp);
	TxCan.TxData[0] |= (DB_VCU.timestamp.time.Hours >= 5 && DB_VCU.timestamp.time.Hours <= 16) << 7;

	// sein value
	TxCan.TxData[1] = iSein_Left << 0;
	TxCan.TxData[1] |= iSein_Right << 1;

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
	CANBUS_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_SWITCH, 8);
	// send message
	return CANBUS_Write(&TxCan);
}

uint8_t CAN_VCU_RTC(void) {
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
	CANBUS_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_RTC, 7);
	// send message
	return CANBUS_Write(&TxCan);
}

uint8_t CAN_VCU_Select_Set(void) {
	static TickType_t tick, tickPeriod;
	static uint8_t iMode_Hide = 0;
	static int8_t iMode_Name = -1;
	static int8_t iMode_Name = -1;

	// Mode Show/Hide Manipulator
	if (DB_VCU.sw.runner.listening) {
		// if mode same
		if (iMode_Name != DB_VCU.sw.runner.mode.val) {
			iMode_Name = DB_VCU.sw.runner.mode.val;
			// reset period tick
			tickPeriod = osKernelSysTick();
		} else if (iMode_Name != DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val]) {
			iMode_Name = DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val];
			// reset period tick
			tickPeriod = osKernelSysTick();
		}

		if ((osKernelSysTick() - tickPeriod) >= tick5000ms ||
				(DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R)) {
			// stop listening
			DB_VCU.sw.runner.listening = 0;
			iMode_Hide = 0;
			iMode_Name = -1;
			iMode_Name = -1;
		} else {
			// blink
			if ((osKernelSysTick() - tick) >= tick250ms) {
				tick = osKernelSysTick();
				iMode_Hide = !iMode_Hide;
			}
		}
	} else {
		iMode_Hide = 0;
	}

	// set message
	TxCan.TxData[0] = DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE];
	TxCan.TxData[0] |= DB_VCU.sw.runner.mode.sub.val[SW_M_TRIP] << 2;
	TxCan.TxData[0] |= DB_VCU.sw.runner.mode.sub.val[SW_M_REPORT] << 3;
	TxCan.TxData[0] |= DB_VCU.sw.runner.mode.val << 4;

	// Send Show/Hide flag
	TxCan.TxData[0] |= iMode_Hide << 6;

	TxCan.TxData[1] = DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_RANGE];
	TxCan.TxData[2] = DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE];

	// dummy algorithm
	if (!DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_RANGE]) {
		DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_RANGE] = 255;
	} else {
		DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_RANGE]--;
	}

	if (DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE] >= 255) {
		DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE] = 0;
	} else {
		DB_VCU.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE]++;
	}

	// set default header
	CANBUS_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_SELECT_SET, 3);
	// send message
	return CANBUS_Write(&TxCan);
}

uint8_t CAN_VCU_Trip_Mode(void) {
	// set message
	TxCan.TxData[0] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 0) & 0xFF;
	TxCan.TxData[1] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 8) & 0xFF;
	TxCan.TxData[2] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 16) & 0xFF;
	TxCan.TxData[3] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 24) & 0xFF;
	TxCan.TxData[4] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 0) & 0xFF;
	TxCan.TxData[5] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 8) & 0xFF;
	TxCan.TxData[6] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 16) & 0xFF;
	TxCan.TxData[7] = (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 24) & 0xFF;

	// dummy algorithm
	if (DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val] == SW_M_TRIP_A) {
		if (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A] >= VCU_ODOMETER_MAX) {
			DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A] = 0;
		} else {
			DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_A]++;
		}
	} else {
		if (DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B] >= VCU_ODOMETER_MAX) {
			DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B] = 0;
		} else {
			DB_VCU.sw.runner.mode.sub.trip[SW_M_TRIP_B]++;
		}
	}
	// set default header
	CANBUS_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_VCU_TRIP_MODE, 8);
	// send message
	return CANBUS_Write(&TxCan);
}

/* ------------------------------------ READER ------------------------------------- */
void CAN_MCU_Dummy_Read(void) {
	uint32_t DB_MCU_RPM;

	// read message
	DB_MCU_RPM = (RxCan.RxData[3] << 24 | RxCan.RxData[2] << 16 | RxCan.RxData[1] << 8 | RxCan.RxData[0]);
	// convert RPM to Speed
	DB_VCU.speed = DB_MCU_RPM * MCU_SPEED_MAX / MCU_RPM_MAX;
}
#endif

