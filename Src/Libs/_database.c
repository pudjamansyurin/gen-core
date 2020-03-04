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
		.timestamp = {
				.time = { 0 },
				.date = { 0 }
		},
		.sw = {
				.count = 6,
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
						.listening = 0,
						.mode = {
								.val = SW_M_DRIVE,
								.sub = {
										.val = {
												SW_M_DRIVE_E,
												SW_M_TRIP_A,
												SW_M_REPORT_RANGE
										},
										.max = {
												SW_M_DRIVE_MAX,
												SW_M_TRIP_MAX,
												SW_M_REPORT_MAX
										},
										.report = { 0, 0 },
										.trip = { 0, 0 }
								}
						}
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
