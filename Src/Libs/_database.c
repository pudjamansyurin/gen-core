/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_database.h"

// @formatter:off
switch_t DB_ECU_Switch[] = {
			{"SELECT", KEY_SELECT_Pin, KEY_SELECT_GPIO_Port, 0},
			{"SET", KEY_SET_Pin, KEY_SET_GPIO_Port, 0},
			{"SEIN LEFT", KEY_SEIN_L_Pin, KEY_SEIN_L_GPIO_Port, 0},
			{"SEIN RIGHT", KEY_SEIN_R_Pin, KEY_SEIN_R_GPIO_Port, 0},
			{"REVERSE", KEY_REVERSE_Pin, KEY_REVERSE_GPIO_Port, 0},
			{"ABS", KEY_ABS_Pin, KEY_ABS_GPIO_Port, 0},
			{"MIRRORING", KEY_MIRROR_Pin, KEY_MIRROR_GPIO_Port, 0},
};

switcher_t DB_HMI_Switcher = {
		SWITCH_MODE_DRIVE,
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
// @formatter:on
switch_timer_t DB_ECU_Switch_Timer[] = { { 0, 0, 0 }, { 0, 0, 0 } };
uint8_t DB_ECU_Switch_Size = sizeof(DB_ECU_Switch) / sizeof(DB_ECU_Switch[0]);

uint8_t DB_ECU_Signal = 0;
uint8_t DB_ECU_Speed = 0;
uint32_t DB_ECU_Odometer = 0;
timestamp_t DB_ECU_TimeStamp;
