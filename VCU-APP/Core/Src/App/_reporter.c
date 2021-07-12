/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_reporter.h"

#include "App/_iap.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t ResponseQueueHandle, ReportQueueHandle;

/* Private constants
 * --------------------------------------------*/
#define RPT_FRAME_FULL_S ((uint8_t)20)
#define RPT_INTERVAL_NORMAL_S ((uint8_t)5)
#define RPT_INTERVAL_BACKUP_S ((uint8_t)20)
#define RPT_INTERVAL_LOST_S ((uint8_t)60)

/* Private types
 * --------------------------------------------*/
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

/* Private variables
 * --------------------------------------------*/
static reporter_t RPT = {
    .block = 0,
    .counter = 0,
    .ovd = {0},
};

static payload_t PLD[PAYLOAD_MAX] = {{.type = PAYLOAD_RESPONSE,
                                      .queue = &ResponseQueueHandle,
                                      .data = &RPT.response,
                                      .pending = 0},
                                     {.type = PAYLOAD_REPORT,
                                      .queue = &ReportQueueHandle,
                                      .data = &RPT.report,
                                      .pending = 0}};

/* Public functions implementation
 * --------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report) {
  report_header_t *h = (report_header_t *)report;
  report_data_t *d = &(report->data);

  memcpy(h->prefix, PREFIX_REPORT, 2);
  h->size = sizeof(h->vin) + sizeof(h->send_time);
  h->vin = IAP_GetBootMeta(VIN_OFFSET);

  // Required data
  h->size += sizeof(d->req);

  d->req.frame_id = frame;
  d->req.log_time = RTC_Read();

  DBG_GetVCU(&(d->req.vcu));
  DBG_GetEEPROM(&(d->req.eeprom));
  DBG_GetGPS(&(d->req.gps));

  // Optional data
  if (frame == FR_FULL) {
    h->size += sizeof(d->opt);

    DBG_GetHBAR(&(d->opt.hbar));
    DBG_GetNET(&(d->opt.net));
    DBG_GetMEMS(&(d->opt.mems));
    DBG_GetRMT(&(d->opt.rmt));
    DBG_GetFGR(&(d->opt.fgr));
    DBG_GetAudio(&(d->opt.audio));

    // NODEs
    DBG_GetHMI1(&(d->opt.hmi1));
    DBG_GetBMS(&(d->opt.bms));
    DBG_GetMCU(&(d->opt.mcu));
    DBG_GetTasks(&(d->opt.task));
  }
}

void RPT_ResponseCapture(response_t *response) {
  report_header_t *h = (report_header_t *)response;

  memcpy(h->prefix, PREFIX_RESPONSE, 2);
  h->size = sizeof(h->vin) + sizeof(h->send_time);
  h->vin = IAP_GetBootMeta(VIN_OFFSET);

  h->size += sizeof(response->header.code) + sizeof(response->header.sub_code) +
             sizeof(response->data.res_code) +
             strnlen(response->data.message, sizeof(response->data.message));
}

FRAME_TYPE RPT_PickFrame(void) {
  FRAME_TYPE frame = FR_FULL;
  uint8_t ovd = RPT.ovd.frame;
  uint8_t simpleCnt = (RPT_FRAME_FULL_S / RPT_INTERVAL_NORMAL_S);

  if (!GATE_ReadPower5v()) {
    frame = FR_FULL;
    RPT.counter = 0;
  } else {
    if (++RPT.counter < simpleCnt)
      frame = FR_SIMPLE;
    else {
      frame = FR_FULL;
      RPT.counter = 0;
    }
  }

  if (ovd) frame = ovd;

  return frame;
}

uint32_t RPT_PickIntervalMS(vehicle_t vehicle) {
  uint8_t ovd = RPT.ovd.interval;
  uint16_t interval = RPT_INTERVAL_NORMAL_S;

  if (vehicle >= VEHICLE_NORMAL)
    interval = RPT_INTERVAL_NORMAL_S;
  else if (vehicle >= VEHICLE_BACKUP)
    interval = RPT_INTERVAL_BACKUP_S;
  else if (vehicle >= VEHICLE_LOST)
    interval = RPT_INTERVAL_LOST_S;

  if (ovd) interval = ovd;

  return interval * 1000;
}

bool RPT_PayloadPending(PAYLOAD_TYPE type) {
  payload_t *pld = &(PLD[type]);
  report_header_t *h = NULL;

  if (RPT.block && pld->type == PAYLOAD_REPORT) return false;

  if (!pld->pending) {
    if (OS_QueueGet(*(pld->queue), pld->data)) {
      h = (report_header_t *)(pld->data);

      pld->size = sizeof(h->prefix) + sizeof(h->size) + h->size;
      pld->pending = true;
    }
  }

  if (pld->pending) {
    h = (report_header_t *)(pld->data);
    h->send_time = RTC_Read();
  }

  return pld->pending;
}

void RPT_IO_SetBlock(uint8_t value) { RPT.block = value; }

void RPT_IO_SetOvdFrame(uint8_t value) { RPT.ovd.frame = value; }

void RPT_IO_SetOvdInterval(uint16_t value) { RPT.ovd.interval = value; }

void RPT_IO_SetPayloadPending(PAYLOAD_TYPE type, uint8_t value) {
  PLD[type].pending = value;
}

void RPT_IO_PayloadDiscard(void) { PLD[PAYLOAD_REPORT].pending = 0; }

const payload_t *RPT_IO_Payload(PAYLOAD_TYPE type) { return &(PLD[type]); }
