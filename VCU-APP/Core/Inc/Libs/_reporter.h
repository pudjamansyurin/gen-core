/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_gyro.h"
#include "Libs/_handlebar.h"
#include "Libs/_utils.h"
#include "Nodes/BMS.h"
#include "Nodes/VCU.h"

/* Exported define
 * -------------------------------------------------------------*/
#define PREFIX_ACK "A@"
#define PREFIX_REPORT "T@"
#define PREFIX_COMMAND "C@"
#define PREFIX_RESPONSE "S@"

/* Exported enum
 * ---------------------------------------------------------------*/
typedef enum {
	FR_SIMPLE = 0,
	FR_FULL = 1,
} FRAME_TYPE;

/* Exported struct
 * --------------------------------------------------------------*/
// header frame (for report & response)
typedef struct __attribute__((packed)) {
	char prefix[2];
	//  uint32_t crc;
	uint8_t size;
	uint32_t vin;
	datetime_t send_time;
} report_header_t;

typedef struct __attribute__((packed)) {
	char prefix[2];
	//  uint32_t crc;
	uint8_t size;
	uint32_t vin;
	datetime_t send_time;
	uint8_t code;
	uint8_t sub_code;
} command_header_t;

typedef struct __attribute__((packed)) {
	int32_t longitude;
	int32_t latitude;
	uint32_t altitude;
	uint8_t hdop;
	uint8_t vdop;
	uint8_t speed;
	uint8_t heading;
	uint8_t sat_in_use;
} gps_debug_t;

typedef struct __attribute__((packed)) {
	uint8_t reverse;
	struct __attribute__((packed)) {
		uint8_t drive;
		uint8_t trip;
		uint8_t report;
	} mode;
	struct __attribute__((packed)) {
		uint32_t a;
		uint32_t b;
		uint32_t odometer;
	} trip;
	struct __attribute__((packed)) {
		uint8_t range;
		uint8_t efficiency;
	} report;
} hbar_debug_t;

typedef struct __attribute__((packed)) {
	uint8_t run;
} hmi1_debug_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t run;
	uint8_t soc;
	uint16_t fault;
	struct __attribute__((packed)) {
		uint32_t id;
		uint16_t voltage;
		uint16_t current;
		uint8_t soc;
		uint16_t temperature;
	} pack[2];
} bms_debug_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t run;
	uint16_t rpm;
	uint8_t speed;
	uint8_t reverse;
	uint16_t temperature;
	uint8_t drive_mode;
	struct {
		uint32_t post;
		uint32_t run;
	} fault;
	struct {
		uint16_t commanded;
		uint16_t feedback;
	} torque;
	struct {
		uint16_t current;
		uint16_t voltage;
	} dcbus;
	struct {
		uint8_t can_mode;
		uint8_t enabled;
		uint8_t lockout;
		uint8_t discharge;
	} inv;
	struct __attribute__((packed)) {
		uint8_t speed_max;
		struct __attribute__((packed)) {
			uint16_t discur_max;
			uint16_t torque_max;
			//	uint8_t rbs_switch;
		} template[HBAR_M_DRIVE_MAX];
	} tpl;
} mcu_debug_t;

typedef struct __attribute__((packed)) {
	tasks_stack_t stack;
	tasks_wakeup_t wakeup;
} tasks_debug_t;

// report frame
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		uint8_t frame_id;
		datetime_t log_time;
		uint8_t driver_id;
		uint16_t events_group;
		int8_t vehicle;
		uint32_t uptime;
	} req;
	struct __attribute__((packed)) {
		uint8_t bat;
		uint8_t signal;
	} opt;
	struct __attribute__((packed)) {
		hbar_debug_t hbar;
		gps_debug_t gps;
		motion_t gyro;
		hmi1_debug_t hmi1;
		bms_debug_t bms;
		mcu_debug_t mcu;
		tasks_debug_t task;
	} debug;
} report_data_t;

typedef struct __attribute__((packed)) {
	report_header_t header;
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
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report);
void RPT_ResponseCapture(response_t *response);
void RPT_FrameDecider(uint8_t backup, FRAME_TYPE *frame);
uint8_t RPT_PayloadPending(payload_t *payload);
uint8_t RPT_WrapPayload(payload_t *payload);
#endif /* REPORTER_H_ */
