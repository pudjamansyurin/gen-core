/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Libs/_gyro.h"
#include "Libs/_handlebar.h"
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"

/* Exported define -------------------------------------------------------------*/
#define PREFIX_REPORT                           "R@"
#define PREFIX_COMMAND                          "C@"
#define PREFIX_RESPONSE                         "S@"

/* Exported enum ---------------------------------------------------------------*/
typedef enum {
	FR_SIMPLE = 0,
	FR_FULL = 1,
} FRAME_TYPE;

/* Exported struct --------------------------------------------------------------*/
// header frame (for report & response)
typedef struct __attribute__((packed)) {
	char prefix[2];
	//  uint32_t crc;
	uint8_t size;
	uint32_t unit_id;
	datetime_t send_time;
} header_t;

typedef struct __attribute__((packed)) {
	char prefix[2];
	//  uint32_t crc;
	uint8_t size;
	uint32_t unit_id;
	datetime_t send_time;
	uint8_t code;
	uint8_t sub_code;
} command_header_t;

// report frame
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		uint8_t frame_id;
		struct __attribute__((packed)) {
			datetime_t log_time;
			uint8_t driver_id;
			uint64_t events_group;
			int8_t vehicle;
			uint32_t uptime;
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
				uint32_t altitude;
				uint8_t hdop;
				uint8_t vdop;
				uint8_t heading;
				uint8_t sat_in_use;
			} gps;
			uint8_t speed;
			uint32_t odometer;
			uint8_t signal;
			uint8_t bat;
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
	struct __attribute__((packed)) {
		rtos_task_t task;
		motion_t motion;
	} debug;
} report_data_t;

typedef struct __attribute__((packed)) {
	header_t header;
	report_data_t data;
} report_t;

// response frame
typedef struct __attribute__((packed)) {
	command_header_t header;
	struct __attribute__((packed)) {
		uint8_t res_code;
		char message[200];
	} data;
} response_t;

// command frame (from server)
typedef struct __attribute__((packed)) {
	command_header_t header;
	struct __attribute__((packed)) {
		char value[200];
	} data;
} command_t;

typedef struct {
	PAYLOAD_TYPE type;
	osMessageQueueId_t *pQueue;
	void *pPayload;
	uint8_t pending;
	uint8_t size;
} payload_t;

/* Public functions prototype ------------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report, vcu_data_t *vcu, bms_data_t *bms, hbar_data_t *hbar);
void RPT_ResponseCapture(response_t *response);
void RPT_FrameDecider(uint8_t backup, FRAME_TYPE *frame);
uint8_t RPT_PayloadPending(payload_t *payload);
uint8_t RPT_WrapPayload(payload_t *payload, uint32_t unit_id);
#endif /* REPORTER_H_ */
