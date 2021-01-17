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
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"

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
      int8_t vehicle;
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
  } test;
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
// typedef struct __attribute__((packed)) {
//   char prefix[2];
//   uint8_t frame_id;
//   uint16_t seq_id;
// } ack_t;

typedef struct {
  PAYLOAD_TYPE type;
  osMessageQueueId_t *pQueue;
  void *pPayload;
  //  uint8_t retry;
  uint8_t pending;
} payload_t;

/* Public functions prototype ------------------------------------------------*/
void RPT_ReportInit(FRAME_TYPE frame, report_t *report, uint16_t *seq_id_report);
void RPT_ResponseInit(response_t *response, uint16_t *seq_id_response);
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report, vcu_data_t *vcu, bms_data_t *bms, hbar_data_t *hbar);
void RPT_ResponseCapture(response_t *response, uint32_t *unit_id);
void RPT_CommandDebugger(command_t *cmd);
FRAME_TYPE RPT_FrameDecider(uint8_t backup);
uint8_t RPT_PacketPending(payload_t *payload);
uint8_t RPT_SendPayload(payload_t *payload);
#endif /* REPORTER_H_ */
