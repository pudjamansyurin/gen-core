/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"
#include "_eeprom.h"
#include "_gps.h"
#include "_rtc.h"
#include "_crc.h"

/* Exported enum ---------------------------------------------------------------*/
typedef enum {
	FR_RESPONSE = 0,
	FR_SIMPLE = 1,
	FR_FULL = 2,
} FRAME_TYPE;

/* Exported struct --------------------------------------------------------------*/
// header frame (for report & response)
typedef struct __attribute__((packed)) {
	char prefix[2];
	uint32_t crc;
	uint8_t size;
	uint8_t frame_id;
	uint32_t unit_id;
	uint16_t seq_id;
} header_t;

// report frame
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		struct __attribute__((packed)) {
			struct __attribute__((packed)) {
				uint64_t send;
				uint64_t log;
			} rtc;
			uint8_t driver_id;
			uint64_t events_group;
		} vcu;
		struct __attribute__((packed)) {
			struct __attribute__((packed)) {
				uint32_t id;
				uint16_t voltage;
				uint16_t current;
			} pack[2];
		} bms;
	} req;
	struct __attribute__((packed)) {
		struct __attribute__((packed)) {
			struct __attribute__((packed)) {
				int32_t longitude;
				int32_t latitude;
				uint8_t hdop;
				uint8_t heading;
			} gps;
			uint8_t speed;
			uint32_t odometer;
			uint8_t signal_percent;
			uint8_t bat_voltage;
			struct __attribute__((packed)) {
				uint8_t range;
				uint8_t efficiency;
			} report;
			struct __attribute__((packed)) {
				uint32_t a;
				uint32_t b;
			} trip;
		} vcu;
		struct __attribute__((packed)) {
			struct __attribute__((packed)) {
				uint16_t soc;
				uint16_t temperature;
			} pack[2];
		} bms;
	} opt;
} report_data_t;

typedef struct __attribute__((packed)) {
	header_t header;
	report_data_t data;
} report_t;

// response frame
typedef struct __attribute__((packed)) {
	header_t header;
	struct __attribute__((packed)) {
		uint8_t code;
		char message[50];
	} data;
} response_t;

// command frame (from server)
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		char prefix[2];
		uint32_t crc;
		uint8_t size;
	} header;
	struct __attribute__((packed)) {
		uint8_t code;
		uint8_t sub_code;
		uint64_t value;
	} data;
} command_t;

// ACK frame (from server)
typedef struct __attribute__((packed)) {
	char prefix[2];
	uint8_t frame_id;
	uint16_t seq_id;
} ack_t;

/* Public functions prototype ------------------------------------------------*/
void Report_Init(FRAME_TYPE frame, report_t *report);
void Response_Init(response_t *response);
void Report_Capture(FRAME_TYPE frame, report_t *report);
void Response_Capture(response_t *response);
void Report_ReCalculate(report_t *report);
#endif /* REPORTER_H_ */
