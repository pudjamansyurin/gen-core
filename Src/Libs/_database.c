/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_database.h"

switch_t DB_VCU_Switch[] = {
		{
				.event = "SELECT",
				.pin = EXT_HBAR_SELECT_Pin,
				.port = EXT_HBAR_SELECT_GPIO_Port,
				.state = 0
		},
		{
				.event = "SET",
				.pin = EXT_HBAR_SET_Pin,
				.port = EXT_HBAR_SET_GPIO_Port,
				.state = 0
		},
		{
				.event = "SEIN LEFT",
				.pin = EXT_HBAR_SEIN_L_Pin,
				.port = EXT_HBAR_SEIN_L_GPIO_Port,
				.state = 0
		},
		{
				.event = "SEIN RIGHT",
				.pin = EXT_HBAR_SEIN_R_Pin,
				.port = EXT_HBAR_SEIN_R_GPIO_Port,
				.state = 0
		},
		{
				.event = "REVERSE",
				.pin = EXT_HBAR_REVERSE_Pin,
				.port = EXT_HBAR_REVERSE_GPIO_Port,
				.state = 0
		},
		{
				.event = "ABS",
				.pin = EXT_ABS_STATUS_Pin,
				.port = EXT_ABS_STATUS_GPIO_Port,
				.state = 0
		},
		{
				.event = "MIRRORING",
				.pin = EXT_HMI2_PHONE_Pin,
				.port = EXT_HMI2_PHONE_GPIO_Port,
				.state = 0
		},
};

switcher_t DB_HMI_Switcher = {
		.mode = SWITCH_MODE_DRIVE,
		.listening = 0,
		.mode_sub = {
				SWITCH_MODE_DRIVE_E,
				SWITCH_MODE_TRIP_A,
				SWITCH_MODE_REPORT_RANGE
		},
		.mode_sub_max = {
				SWITCH_MODE_DRIVE_MAX,
				SWITCH_MODE_TRIP_MAX,
				SWITCH_MODE_REPORT_MAX
		},
		.mode_sub_report = {
				0, 0
		},
		.mode_sub_trip = {
				0, 0
		}
};

switch_timer_t DB_VCU_Switch_Timer[] = {
		{
				.start = 0,
				.running = 0,
				.time = 0
		},
		{
				.start = 0,
				.running = 0,
				.time = 0
		}
};

status_t DB_HMI_Status = {
		.lamp = 1,
		.warning = 1,
		.temperature = 1,
		.finger = 1,
		.keyless = 1
};

uint8_t DB_VCU_Switch_Count = sizeof(DB_VCU_Switch) / sizeof(DB_VCU_Switch[0]);

uint8_t DB_VCU_Signal = 0;
uint8_t DB_VCU_Speed = 0;
uint32_t DB_VCU_Odometer = 0;
timestamp_t DB_VCU_TimeStamp;
