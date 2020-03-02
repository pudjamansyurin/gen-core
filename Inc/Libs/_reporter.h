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

/*  typedef -----------------------------------------------------------*/
typedef enum {
	FRAME_RESPONSE = 0,
	FRAME_SIMPLE = 1,
	FRAME_FULL = 2,
} frame_t;

/*  typedef struct -----------------------------------------------------------*/
// header frame (for report & response)
typedef struct __attribute__((packed)) {
	uint16_t prefix;
	uint32_t crc;
	uint8_t size;
	uint8_t frame_id;
	uint32_t unit_id;
	uint16_t seq_id;
} report_header_t;

// report frame
typedef struct __attribute__((packed)) {
	int32_t longitude;
	int32_t latitude;
	uint8_t hdop;
	uint8_t heading;
} report_data_gps_t;

typedef struct __attribute__((packed)) {
	uint64_t rtc_send_datetime;
	uint64_t rtc_log_datetime;
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
} report_t;

// response frame
typedef struct __attribute__((packed)) {
	uint8_t code;
	char message[50];
} response_data_t;

typedef struct __attribute__((packed)) {
	report_header_t header;
	response_data_t data;
} response_t;

// command frame (from server)
typedef struct __attribute__((packed)) {
	uint16_t prefix;
	uint32_t crc;
	uint8_t size;
} command_header_t;

typedef struct __attribute__((packed)) {
	uint8_t code;
	uint8_t sub_code;
	uint64_t value;
} command_data_t;

typedef struct __attribute__((packed)) {
	command_header_t header;
	command_data_t data;
} command_t;

// ACK frame (from server)
typedef struct __attribute__((packed)) {
	uint16_t prefix;
	uint8_t frame_id;
	uint16_t seq_id;
} ack_t;

// public function
void Reporter_Reset(frame_t frame);
void Reporter_Set_UnitID(uint32_t unitId);
void Reporter_Set_Odometer(uint32_t odom);
void Reporter_Set_GPS(gps_t *hgps);
void Reporter_Set_Speed(gps_t *hgps);
void Reporter_Set_Event(uint64_t event_id, uint8_t bool);
uint8_t Reporter_Read_Event(uint64_t event_id);
void Reporter_Capture(frame_t frame);

#endif /* REPORTER_H_ */
