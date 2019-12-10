/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_canbus.h"
#include "_rtc.h"

extern CAN_Rx RxCan;

// ==================================== ECU =========================================
#if (CAN_NODE & CAN_NODE_ECU)
uint8_t CANBUS_ECU_Switch(void) {
	CAN_Tx TxCan;
	const TickType_t tick500ms = pdMS_TO_TICKS(500);
	static TickType_t tick, tickSein;
	static uint8_t Sein_Left_Internal = 0, Sein_Right_Internal = 0;
	static status_t DB_HMI_Status_Internal;
	extern switch_t DB_ECU_Switch[];
	extern status_t DB_HMI_Status;
	extern timestamp_t DB_ECU_TimeStamp;
	extern uint32_t DB_ECU_Odometer;
	extern uint8_t DB_ECU_Signal;

	// indicator manipulator
	if ((osKernelSysTick() - tick) >= tick500ms) {
		// finger
		if (DB_HMI_Status.finger) {
			tick = osKernelSysTick();
			DB_HMI_Status_Internal.finger = !DB_HMI_Status_Internal.finger;
		}
		// keyless
		if (DB_HMI_Status.keyless) {
			tick = osKernelSysTick();
			DB_HMI_Status_Internal.keyless = !DB_HMI_Status_Internal.keyless;
		}
		// temperature
		if (DB_HMI_Status.temperature) {
			tick = osKernelSysTick();
			DB_HMI_Status_Internal.temperature = !DB_HMI_Status_Internal.temperature;
		}
	}

	// sein manipulator
	if ((osKernelSysTick() - tickSein) >= tick500ms) {
		if (DB_ECU_Switch[IDX_KEY_SEIN_LEFT].state && DB_ECU_Switch[IDX_KEY_SEIN_RIGHT].state) {
			// hazard
			tickSein = osKernelSysTick();
			Sein_Left_Internal = !Sein_Left_Internal;
			Sein_Right_Internal = Sein_Left_Internal;
		} else if (DB_ECU_Switch[IDX_KEY_SEIN_LEFT].state) {
			// left sein
			tickSein = osKernelSysTick();
			Sein_Left_Internal = !Sein_Left_Internal;
			Sein_Right_Internal = 0;
		} else if (DB_ECU_Switch[IDX_KEY_SEIN_RIGHT].state) {
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
	TxCan.TxData[0] = DB_ECU_Switch[IDX_KEY_ABS].state;
	TxCan.TxData[0] |= DB_ECU_Switch[IDX_KEY_MIRRORING].state << 1;
	TxCan.TxData[0] |= 1 << 2;
	TxCan.TxData[0] |= 1 << 3;
	TxCan.TxData[0] |= DB_HMI_Status_Internal.temperature << 4;
	TxCan.TxData[0] |= DB_HMI_Status_Internal.finger << 5;
	TxCan.TxData[0] |= DB_HMI_Status_Internal.keyless << 6;
	// check daylight (for auto brightness of HMI)
	RTC_Read_RAW(&DB_ECU_TimeStamp);
	TxCan.TxData[0] |= (DB_ECU_TimeStamp.time.Hours >= 5 && DB_ECU_TimeStamp.time.Hours <= 16) << 7;

	// sein value
	TxCan.TxData[1] = Sein_Left_Internal << 0;
	TxCan.TxData[1] |= Sein_Right_Internal << 1;

	// signal strength
	TxCan.TxData[2] = DB_ECU_Signal;

	// odometer
	TxCan.TxData[4] = (DB_ECU_Odometer & 0x000000FF);
	TxCan.TxData[5] = (DB_ECU_Odometer & 0x0000FF00) >> 8;
	TxCan.TxData[6] = (DB_ECU_Odometer & 0x00FF0000) >> 16;
	TxCan.TxData[7] = (DB_ECU_Odometer & 0xFF000000) >> 24;

	// dummy algorithm
	DB_ECU_Signal = (DB_ECU_Signal > 100 ? 0 : DB_ECU_Signal + 1);
	DB_ECU_Odometer = (DB_ECU_Odometer >= ECU_ODOMETER_MAX ? 0 : (DB_ECU_Odometer + 1));

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_ECU_SWITCH, 8);
	// send message
	return CAN_Write(&TxCan);
}

uint8_t CANBUS_ECU_RTC(void) {
	CAN_Tx TxCan;
	extern timestamp_t DB_ECU_TimeStamp;

	// set message
	RTC_Read_RAW(&DB_ECU_TimeStamp);
	TxCan.TxData[0] = DB_ECU_TimeStamp.time.Seconds;
	TxCan.TxData[1] = DB_ECU_TimeStamp.time.Minutes;
	TxCan.TxData[2] = DB_ECU_TimeStamp.time.Hours;
	TxCan.TxData[3] = DB_ECU_TimeStamp.date.Date;
	TxCan.TxData[4] = DB_ECU_TimeStamp.date.Month;
	TxCan.TxData[5] = DB_ECU_TimeStamp.date.Year;
	TxCan.TxData[7] = DB_ECU_TimeStamp.date.WeekDay;

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_ECU_RTC, 8);
	// send message
	return CAN_Write(&TxCan);
}

uint8_t CANBUS_ECU_Select_Set(void) {
	CAN_Tx TxCan;
	const TickType_t tick250ms = pdMS_TO_TICKS(250);
	const TickType_t tick5000ms = pdMS_TO_TICKS(5000);
	static TickType_t tick, tickPeriod;
	static uint8_t Mode_Hide_Internal = 0;
	static int8_t Mode_Name_Internal = -1;
	static int8_t Mode_Value_Internal = -1;
	extern switch_t DB_ECU_Switch[];
	extern switcher_t DB_HMI_Switcher;

	// Mode Show/Hide Manipulator
	if (DB_HMI_Switcher.listening) {
		// if mode same
		if (Mode_Name_Internal != DB_HMI_Switcher.mode) {
			Mode_Name_Internal = DB_HMI_Switcher.mode;
			// reset period tick
			tickPeriod = osKernelSysTick();
		} else if (Mode_Value_Internal != DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]) {
			Mode_Value_Internal = DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode];
			// reset period tick
			tickPeriod = osKernelSysTick();
		}

		if ((osKernelSysTick() - tickPeriod) >= tick5000ms ||
				(DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] == SWITCH_MODE_DRIVE_R)) {
			// stop listening
			DB_HMI_Switcher.listening = 0;
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
	TxCan.TxData[0] = DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE];
	TxCan.TxData[0] |= DB_HMI_Switcher.mode_sub[SWITCH_MODE_TRIP] << 2;
	TxCan.TxData[0] |= DB_HMI_Switcher.mode_sub[SWITCH_MODE_REPORT] << 3;
	TxCan.TxData[0] |= DB_HMI_Switcher.mode << 4;

	// Send Show/Hide flag
	TxCan.TxData[0] |= Mode_Hide_Internal << 6;

	TxCan.TxData[1] = DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_RANGE];
	TxCan.TxData[2] = DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_AVERAGE];

	// dummy algorithm
	if (!DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_RANGE]) {
		DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_RANGE] = 255;
	} else {
		DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_RANGE]--;
	}

	if (DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_AVERAGE] >= 255) {
		DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_AVERAGE] = 0;
	} else {
		DB_HMI_Switcher.mode_sub_report[SWITCH_MODE_REPORT_AVERAGE]++;
	}

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_ECU_SELECT_SET, 3);
	// send message
	return CAN_Write(&TxCan);
}

uint8_t CANBUS_ECU_Trip_Mode(void) {
	CAN_Tx TxCan;
	extern switcher_t DB_HMI_Switcher;

	// set message
	TxCan.TxData[0] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_A] & 0x000000FF);
	TxCan.TxData[1] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_A] & 0x0000FF00) >> 8;
	TxCan.TxData[2] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_A] & 0x00FF0000) >> 16;
	TxCan.TxData[3] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_A] & 0xFF000000) >> 24;
	TxCan.TxData[4] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_B] & 0x000000FF);
	TxCan.TxData[5] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_B] & 0x0000FF00) >> 8;
	TxCan.TxData[6] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_B] & 0x00FF0000) >> 16;
	TxCan.TxData[7] = (DB_HMI_Switcher.mode_sub_trip[SWITCH_MODE_TRIP_B] & 0xFF000000) >> 24;

	// dummy algorithm
	if (DB_HMI_Switcher.mode_sub_trip[DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]] >= ECU_ODOMETER_MAX) {
		DB_HMI_Switcher.mode_sub_trip[DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]] = 0;
	} else {
		DB_HMI_Switcher.mode_sub_trip[DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]]++;
	}

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_ECU_TRIP_MODE, 8);
	// send message
	return CAN_Write(&TxCan);
}

/* ------------------------------------ READER ------------------------------------- */
void CANBUS_MCU_Dummy_Read(void) {
	extern uint8_t DB_ECU_Speed;
	uint32_t DB_MCU_RPM;

	// read message
	DB_MCU_RPM = (RxCan.RxData[3] << 24 | RxCan.RxData[2] << 16 | RxCan.RxData[1] << 8 | RxCan.RxData[0]);
	// convert RPM to Speed
	DB_ECU_Speed = DB_MCU_RPM * MCU_SPEED_MAX / MCU_RPM_MAX;
}
#endif

// ==================================== MCU =========================================
#if (CAN_NODE & CAN_NODE_MCU)
uint8_t CANBUS_MCU_Dummy(void) {
	CAN_Tx TxCan;
	extern uint16_t MCU_RPM;
	static uint8_t MCU_Temperature = 0;

	// set message
	// RPM data
	TxCan.TxData[0] = (MCU_RPM & 0x00FF);
	TxCan.TxData[1] = (MCU_RPM & 0xFF00) >> 8;
	// Temperature data
	TxCan.TxData[2] = MCU_Temperature;

	// dummy algorithm
	MCU_Temperature++;

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_MCU_DUMMY, 3);

	// send message
	return CAN_Write(&TxCan);
}
#endif

// ==================================== BMS =========================================
#if (CAN_NODE & CAN_NODE_BMS)
uint8_t CANBUS_BMS_Dummy(void) {
	CAN_Tx TxCan;
	static uint8_t BMS_SoC = 100;
	static uint8_t BMS_Temperature = 0;

	// set message
	// SoC data
	TxCan.TxData[0] = BMS_SoC;
	// Temperature data
	TxCan.TxData[1] = BMS_Temperature++;

	// dummy algorithm
	BMS_SoC = (!BMS_SoC ? 100 : (BMS_SoC - 1));

	// set default header
	CAN_Set_Tx_Header(&(TxCan.TxHeader), CAN_ADDR_BMS_DUMMY, 2);

	// send message
	return CAN_Write(&TxCan);
}
#endif
