/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_mems.h"
#include "Libs/_hbar.h"
#include "Libs/_utils.h"
#include "Nodes/BMS.h"
#include "Nodes/VCU.h"
#include "Nodes/MCU.h"

/* Exported define
 * -------------------------------------------------------------*/
#define RPT_FRAME_FULL (uint8_t)20      // in second
#define RPT_INTERVAL_NORMAL (uint8_t)5  // in second
#define RPT_INTERVAL_BACKUP (uint8_t)20 // in second
#define RPT_INTERVAL_LOST (uint8_t)60   // in second

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

typedef struct __attribute__((packed)) {
	uint8_t reverse;
	struct __attribute__((packed)) {
		uint8_t drive;
		uint8_t trip;
		uint8_t report;
	} mode;
	struct __attribute__((packed)) {
		uint16_t a;
		uint16_t b;
		uint16_t odometer;
	} trip;
	struct __attribute__((packed)) {
		uint8_t range;
		uint8_t efficiency;
	} report;
} hbar_report_t;

typedef struct __attribute__((packed)) {
	uint8_t signal;
	int8_t state;
	int8_t ipstatus;
} net_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	int32_t longitude;
	int32_t latitude;
	uint32_t altitude;
	uint8_t hdop;
	uint8_t vdop;
	uint8_t speed;
	uint8_t heading;
	uint8_t sat_in_use;
} gps_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t detector;
	struct __attribute__((packed)) {
		int16_t x;
		int16_t y;
		int16_t z;
	} accel;
	struct __attribute__((packed)) {
		int16_t x;
		int16_t y;
		int16_t z;
	} gyro;
	struct __attribute__((packed)) {
		int16_t pitch;
		int16_t roll;
	} ypr;
	struct __attribute__((packed)) {
		uint16_t accelerometer;
		uint16_t gyroscope;
		uint16_t tilt;
		uint16_t temperature;
	} total;
} mems_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t nearby;
} remote_report_t;

typedef struct __attribute__((packed)) {
	uint8_t verified;
	uint8_t driver_id;
} finger_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t mute;
	uint8_t volume;
} audio_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
} hmi1_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t run;
	uint16_t fault;
	uint8_t soc;
	struct __attribute__((packed)) {
		uint32_t id;
		uint16_t fault;
		uint16_t voltage;
		uint16_t current;
		uint8_t soc;
		uint16_t temperature;
	} pack[2];
} bms_report_t;

typedef struct __attribute__((packed)) {
	uint8_t active;
	uint8_t run;
	uint8_t reverse;
	uint8_t drive_mode;
	int16_t rpm;
	uint8_t speed;
	int16_t temperature;
	struct {
		uint32_t post;
		uint32_t run;
	} fault;
	struct {
		int16_t commanded;
		int16_t feedback;
	} torque;
	struct {
		int16_t current;
		int16_t voltage;
	} dcbus;
	struct {
		uint8_t enabled;
		uint8_t lockout;
		uint8_t discharge;
	} inv;
	struct __attribute__((packed)) {
		int16_t rpm_max;
		uint8_t speed_max;
		mcu_template_t tpl[HBAR_M_DRIVE_MAX];
	} par;
} mcu_report_t;

typedef struct __attribute__((packed)) {
	tasks_stack_t stack;
	tasks_wakeup_t wakeup;
} tasks_report_t;

// report frame
typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
		uint8_t frame_id;
		datetime_t log_time;
		uint16_t events_group;
		int8_t vehicle;
		uint32_t uptime;
		uint8_t buffered;
		uint8_t bat;
	} req;
	struct __attribute__((packed)) {
		hbar_report_t hbar;

		net_report_t net;
		gps_report_t gps;
		mems_report_t mems;
		remote_report_t rmt;
		finger_report_t fgr;
		audio_report_t audio;

		hmi1_report_t hmi1;
		bms_report_t bms;
		mcu_report_t mcu;
		tasks_report_t task;
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
uint16_t RPT_IntervalDecider(void);
uint8_t RPT_PayloadPending(payload_t *payload);
uint8_t RPT_WrapPayload(payload_t *payload);
#endif /* REPORTER_H_ */
