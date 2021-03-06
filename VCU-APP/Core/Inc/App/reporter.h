/*
 * reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__REPORTER_H_
#define INC_APP__REPORTER_H_

/* Includes
 * --------------------------------------------*/
#include "App/command.h"
#include "App/debugger.h"

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

typedef enum {
  PAYLOAD_RESPONSE = 0,
  PAYLOAD_REPORT,
  PAYLOAD_MAX = 2,
} PAYLOAD_TYPE;

/* Exported types
 * --------------------------------------------*/
typedef struct {
  PAYLOAD_TYPE type;
  osMessageQueueId_t *queue;
  void *data;
  bool pending;
  uint8_t size;
} payload_t;

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
    ee_dbg_t eeprom;
    gps_dbg_t gps;
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

/* Public functions prototype
 * --------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report);
void RPT_ResponseCapture(response_t *response);
FRAME_TYPE RPT_PickFrame(void);
uint32_t RPT_PickIntervalMS(vehicle_t vehicle);
bool RPT_PayloadPending(PAYLOAD_TYPE type);

void RPT_IO_SetBlock(uint8_t value);
void RPT_IO_SetOvdFrame(uint8_t value);
void RPT_IO_SetOvdInterval(uint16_t value);
void RPT_IO_SetPayloadPending(PAYLOAD_TYPE type, uint8_t value);
void RPT_IO_PayloadDiscard(void);

const payload_t *RPT_IO_Payload(PAYLOAD_TYPE type);
#endif /* INC_APP__REPORTER_H_ */
