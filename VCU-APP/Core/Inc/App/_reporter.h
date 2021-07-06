/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__REPORTER_H_
#define INC_APP__REPORTER_H_

/* Includes
 * --------------------------------------------*/
#include "App/_command.h"
#include "App/_debugger.h"

/* Exported constants
 * --------------------------------------------*/
#define PREFIX_ACK "A@"
#define PREFIX_REPORT "T@"
#define PREFIX_COMMAND "C@"
#define PREFIX_RESPONSE "S@"

/* Exported enums
 * --------------------------------------------*/
typedef enum {
	FR_SIMPLE = 1,
	FR_FULL = 2,
} FRAME_TYPE;

/* Exported structs
 * --------------------------------------------*/
// header frame (for report & response)
typedef struct __attribute__((packed)) {
	char prefix[2];
	uint8_t size;
	uint32_t vin;
	datetime_t send_time;
} report_header_t;

// report frame
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		uint8_t frame_id;
		datetime_t log_time;
		vcu_dbg_t vcu;
		gps_dbg_t gps;
		ee_dbg_t eeprom;
	} req;
	struct __attribute__((packed)) {
		hbar_dbg_t hbar;
		net_dbg_t net;
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

typedef struct {
	PAYLOAD_TYPE type;
	osMessageQueueId_t *queue;
	void *data;
	bool pending;
	uint8_t size;
} payload_t;

typedef struct {
	uint8_t block;
	uint8_t counter;
	struct {
		uint16_t interval;
		uint8_t frame;
	} ovd;
	report_t report;
	response_t response;
} reporter_t;


/* Public functions prototype
 * --------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report);
void RPT_ResponseCapture(response_t *response);
FRAME_TYPE RPT_FrameDecider(void);
uint32_t RPT_IntervalDeciderMS(vehicle_state_t state);
bool RPT_PayloadPending(PAYLOAD_TYPE type);

void RPT_IO_SetBlock(uint8_t value);
void RPT_IO_SetOvdFrame(uint8_t value);
void RPT_IO_SetOvdInterval(uint16_t value);
void RPT_IO_SetPayloadPending(PAYLOAD_TYPE type, uint8_t value);
void RPT_IO_PayloadDiscard(void);

payload_t RPT_IO_GetPayload(PAYLOAD_TYPE type);
#endif /* INC_APP__REPORTER_H_ */
