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
#include "Libs/_debugger.h"
#include "Drivers/_rtc.h"

/* Exported define
 * -------------------------------------------------------------*/
#define RPT_FRAME_FULL_S ((uint8_t)20)      
#define RPT_INTERVAL_NORMAL_S ((uint8_t)5) 
#define RPT_INTERVAL_BACKUP_S ((uint8_t)20)
#define RPT_INTERVAL_LOST_S ((uint8_t)60)

#define PREFIX_ACK "A@"
#define PREFIX_REPORT "T@"
#define PREFIX_COMMAND "C@"
#define PREFIX_RESPONSE "S@"

/* Exported enum
 * ---------------------------------------------------------------*/
typedef enum {
	FR_SIMPLE = 1,
	FR_FULL = 2,
} FRAME_TYPE;

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
	struct {
		uint16_t interval;
		uint8_t frame;
	} override;
} reporter_t;

// header frame (for report & response)
typedef struct __attribute__((packed)) {
	char prefix[2];
	uint8_t size;
	uint32_t vin;
	datetime_t send_time;
} report_header_t;

typedef struct __attribute__((packed)) {
	char prefix[2];
	uint8_t size;
	uint32_t vin;
	datetime_t send_time;
	uint8_t code;
	uint8_t sub_code;
} command_header_t;

// report frame
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		uint8_t frame_id;
		datetime_t log_time;
		vcu_dbg_t vcu;
	} req;
	struct __attribute__((packed)) {
		hbar_dbg_t hbar;
		net_dbg_t net;
		gps_dbg_t gps;
		mems_dbg_t mems;
		remote_dbg_t rmt;
		finger_dbg_t fgr;
		audio_dbg_t audio;
		hmi1_dbg_t hmi1;
		bms_dbg_t bms;
		mcu_dbg_t mcu;
		tasks_dbg_t task;
	} opt;
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

/* Exported variables
 * ----------------------------------------------------------*/
extern reporter_t RPT;

/* Public functions prototype ------------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report);
void RPT_ResponseCapture(response_t *response);
FRAME_TYPE RPT_FrameDecider(void);
uint16_t RPT_IntervalDecider(vehicle_state_t state);
uint8_t RPT_PayloadPending(payload_t *payload);
uint8_t RPT_WrapPayload(payload_t *payload);
#endif /* REPORTER_H_ */
