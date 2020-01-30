/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

#include <stdio.h>											// for: sprintf()
#include "_config.h"
#include "_flash.h"
#include "_ee_emulation.h"
#include "_nmea.h"
#include "_rtc.h"

// Events group id (max 64 events, uint64_t)
#define REPORT_BIKE_FALLING 						SetBit(0)
#define REPORT_BIKE_CRASHED 						SetBit(1)
#define REPORT_KEYLESS_MISSING					SetBit(2)
#define REPORT_SIMCOM_RESTART						SetBit(3)

/*  typedef enum -----------------------------------------------------------*/
typedef enum {
	FRAME_RESPONSE = 0,
	FRAME_SIMPLE = 1,
	FRAME_FULL = 2,
} frame_t;

/*  typedef struct -----------------------------------------------------------*/
// report frame
typedef struct __attribute__((packed)) {
	uint16_t prefix;
	uint16_t crc;
	uint16_t size;
	uint8_t frame_id;
	uint32_t unit_id;
} report_header_t;

typedef struct __attribute__((packed)) {
	int32_t longitude;
	int32_t latitude;
	uint8_t hdop;
	uint8_t heading;
} report_data_gps_t;

typedef struct __attribute__((packed)) {
	uint16_t seq_id;
	uint64_t rtc_datetime;
	uint8_t driver_id;
	uint64_t events_group;
	uint8_t speed;
} report_data_req_t;

typedef struct __attribute__((packed)) {
	report_data_gps_t gps;
	uint32_t odometer;
	uint8_t bat_voltage;
	uint8_t report_range;
	uint8_t report_battery;
	uint32_t trip_a;
	uint32_t trip_b;
} report_data_opt_t;

typedef struct __attribute__((packed)) {
	report_data_req_t req;
	report_data_opt_t opt;
} report_data_t;

typedef struct __attribute__((packed)) {
	report_header_t header;
	report_data_t data;
//	uint8_t terminator;
} report_t;

// response frame
typedef struct __attribute__((packed)) {
	char message[50];
} response_data_t;

typedef struct __attribute__((packed)) {
	report_header_t header;
	response_data_t data;
//	uint8_t terminator;
} response_t;

// public function
void Reporter_Reset(frame_t frame);
void Reporter_Set_Prefix(uint16_t prefix);
void Reporter_Set_UnitID(uint64_t unitId);
void Reporter_Set_Odometer(uint32_t odom);
void Reporter_Set_GPS(gps_t *hgps);
void Reporter_Set_Speed(gps_t *hgps);
void Reporter_Set_Event(uint64_t event_id, uint8_t bool);
void Reporter_Set_Header(frame_t frame);

#endif /* REPORTER_H_ */
