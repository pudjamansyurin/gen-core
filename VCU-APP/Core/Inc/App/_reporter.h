/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

/* Includes
 * --------------------------------------------*/
#include "App/_command.h"
#include "App/_debugger.h"
#include "Libs/_utils.h"

/* Exported constants
 * --------------------------------------------*/
#define RPT_FRAME_FULL_S ((uint8_t)20)
#define RPT_INTERVAL_NORMAL_S ((uint8_t)5)
#define RPT_INTERVAL_BACKUP_S ((uint8_t)20)
#define RPT_INTERVAL_LOST_S ((uint8_t)60)

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

typedef struct {
  PAYLOAD_TYPE type;
  osMessageQueueId_t *queue;
  void *data;
  uint8_t pending;
  uint8_t size;
} payload_t;

typedef struct {
  uint8_t block;
  struct {
    uint16_t interval;
    uint8_t frame;
  } override;
  payload_t payloads[PAYLOAD_MAX];
} reporter_t;

/* Exported variables
 * --------------------------------------------*/
extern reporter_t RPT;

/* Public functions prototype
 * --------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report);
void RPT_ResponseCapture(response_t *response);
FRAME_TYPE RPT_FrameDecider(void);
uint32_t RPT_IntervalDeciderMS(vehicle_state_t state);
uint8_t RPT_PayloadPending(payload_t *payload);
#endif /* REPORTER_H_ */
