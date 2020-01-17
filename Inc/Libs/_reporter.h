/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

#include <stdio.h>							// for: sprintf()
#include "_config.h"
#include "_flash.h"
#include "_ee_emulation.h"
#include "_nmea.h"
#include "_rtc.h"

//#define REPORT_MESSAGE_LENGTH				500

// Events group id (max 64 events, uint64_t)
#define REPORT_BIKE_FALLING 						SetBit(0)
#define REPORT_BIKE_CRASHED 						SetBit(1)

/*  typedef -----------------------------------------------------------*/
typedef struct __attribute__((packed)) {
	int32_t longitude;
	int32_t latitude;
	uint8_t hdop;
	uint8_t heading;
} report_data_gps_t;

typedef struct __attribute__((packed)) {
	uint64_t rtc_datetime;
	uint8_t driver_id;
	uint64_t events_group;
	report_data_gps_t gps;
	uint8_t speed;
	uint32_t odometer;
	uint8_t backup_bat_voltage;
//	char message[REPORT_MESSAGE_LENGTH + 1];
} report_data_t;

typedef struct __attribute__((packed)) {
	uint16_t prefix;
	uint16_t crc;
	uint16_t length;
	uint16_t seq_id;
	uint8_t frame_id;
	uint64_t unit_id;
} report_header_t;

typedef struct __attribute__((packed)) {
	report_header_t header;
	report_data_t data;
	uint8_t terminator;
} report_t;

// public function
void Reporter_Reset(void);
void Reporter_Set_Odometer(uint32_t odom);
void Reporter_Set_GPS(gps_t *hgps);
void Reporter_Set_Event(uint64_t event_id, uint8_t bool);
void Reporter_Set_Frame(void);

#endif /* REPORTER_H_ */
