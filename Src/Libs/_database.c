/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_database.h"

switch_t DB_VCU_Switch[] = {
		{ "SELECT", EXT_HBAR_SELECT_Pin, EXT_HBAR_SELECT_GPIO_Port, 0 },
		{ "SET", EXT_HBAR_SET_Pin, EXT_HBAR_SET_GPIO_Port, 0 },
		{ "SEIN LEFT", EXT_HBAR_SEIN_L_Pin, EXT_HBAR_SEIN_L_GPIO_Port, 0 },
		{ "SEIN RIGHT", EXT_HBAR_SEIN_R_Pin, EXT_HBAR_SEIN_R_GPIO_Port, 0 },
		{ "REVERSE", EXT_HBAR_REVERSE_Pin, EXT_HBAR_REVERSE_GPIO_Port, 0 },
		{ "ABS", EXT_ABS_STATUS_Pin, EXT_ABS_STATUS_GPIO_Port, 0 },
		{ "MIRRORING", EXT_HMI2_PHONE_Pin, EXT_HMI2_PHONE_GPIO_Port, 0 },
};

switcher_t DB_HMI_Switcher = {
		SWITCH_MODE_DRIVE,
		0,
		{
				SWITCH_MODE_DRIVE_E,
				SWITCH_MODE_TRIP_A,
				SWITCH_MODE_REPORT_RANGE
		},
		{
				SWITCH_MODE_DRIVE_MAX,
				SWITCH_MODE_TRIP_MAX,
				SWITCH_MODE_REPORT_MAX
		},
		{
				0, 0
		},
		{
				0, 0
		}
};
status_t DB_HMI_Status = { 1, 1, 1, 1, 1 };

switch_timer_t DB_VCU_Switch_Timer[] = { { 0, 0, 0 }, { 0, 0, 0 } };
uint8_t DB_VCU_Switch_Size = sizeof(DB_VCU_Switch) / sizeof(DB_VCU_Switch[0]);

uint8_t DB_VCU_Signal = 0;
uint8_t DB_VCU_Speed = 0;
uint32_t DB_VCU_Odometer = 0;
timestamp_t DB_VCU_TimeStamp;
