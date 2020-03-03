/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_database.h"

vcu_t DB_VCU = {
		.signal = 0,
		.speed = 0,
		.odometer = 0,
		.timestamp = { 0 },
		.sw = {
				.list = {
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
						}
				},
				.timer = {
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
				},
				.runner = {
						.mode = {
								.val = SWITCH_MODE_DRIVE,
								.sub = {
										.val = {
												SWITCH_MODE_DRIVE_E,
												SWITCH_MODE_TRIP_A,
												SWITCH_MODE_REPORT_RANGE
										},
										.max = {
												SWITCH_MODE_DRIVE_MAX,
												SWITCH_MODE_TRIP_MAX,
												SWITCH_MODE_REPORT_MAX
										},
										.report = { 0 },
										.trip = { 0 }
								}
						},
						.listening = 0,
				}
		}
};

hmi1_t DB_HMI1 = {
		.status = {
				.lamp = 1,
				.warning = 1,
				.temperature = 1,
				.finger = 1,
				.keyless = 1
		}
};

hmi2_t DB_HMI2 = {
		.shutdown = 0
};
