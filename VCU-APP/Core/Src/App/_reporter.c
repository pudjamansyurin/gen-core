/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_reporter.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t ResponseQueueHandle, ReportQueueHandle;

/* Private variables
 * --------------------------------------------*/
static report_t report;
static response_t response;

/* Public variables
 * --------------------------------------------*/
reporter_t RPT = {.block = 0,
                  .override = {0},
                  .payloads = {{.type = PAYLOAD_RESPONSE,
                                .queue = &ResponseQueueHandle,
                                .data = &response,
                                .pending = 0},
                               {.type = PAYLOAD_REPORT,
                                .queue = &ReportQueueHandle,
                                .data = &report,
                                .pending = 0}}};

/* Public functions implementation
 * --------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report) {
  report_header_t *header = (report_header_t *)report;
  report_data_t *d = &(report->data);

  memcpy(header->prefix, PREFIX_REPORT, 2);
  header->size = sizeof(header->vin) + sizeof(header->send_time);
  header->vin = VIN_VALUE;

  // Required data
  header->size += sizeof(d->req);

  d->req.frame_id = frame;
  d->req.log_time = RTC_Read();

  DBG_GetVCU(&(d->req.vcu));
  DBG_GetGPS(&(d->req.gps));

  // Optional data
  if (frame == FR_FULL) {
    header->size += sizeof(d->opt);

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
  report_header_t *header = (report_header_t *)response;

  memcpy(header->prefix, PREFIX_RESPONSE, 2);
  header->size = sizeof(header->vin) + sizeof(header->send_time);
  header->vin = VIN_VALUE;

  header->size +=
      sizeof(response->header.code) + sizeof(response->header.sub_code) +
      sizeof(response->data.res_code) +
      strnlen(response->data.message, sizeof(response->data.message));
}

FRAME_TYPE RPT_FrameDecider(void) {
  static uint8_t counter = 0;
  FRAME_TYPE frame = FR_FULL;
  uint8_t override = RPT.override.frame;
  uint8_t simpleCnt = (RPT_FRAME_FULL_S / RPT_INTERVAL_NORMAL_S);

  if (!GATE_ReadPower5v()) {
    frame = FR_FULL;
    counter = 0;
  } else {
    if (++counter < simpleCnt)
      frame = FR_SIMPLE;
    else {
      frame = FR_FULL;
      counter = 0;
    }
  }

  if (override) frame = override;

  return frame;
}

uint32_t RPT_IntervalDeciderMS(vehicle_state_t state) {
  uint8_t override = RPT.override.interval;
  uint16_t interval = RPT_INTERVAL_NORMAL_S;

  if (state >= VEHICLE_NORMAL)
    interval = RPT_INTERVAL_NORMAL_S;
  else if (state >= VEHICLE_BACKUP)
    interval = RPT_INTERVAL_BACKUP_S;
  else if (state >= VEHICLE_LOST)
    interval = RPT_INTERVAL_LOST_S;

  if (override) interval = override;

  return interval * 1000;
}

bool RPT_PayloadPending(payload_t *payload) {
  report_header_t *header = NULL;

  if (RPT.block && payload->type == PAYLOAD_REPORT) return false;

  if (!payload->pending) {
    if (_osQueueGet(*(payload->queue), payload->data)) {
      header = (report_header_t *)(payload->data);

      payload->size =
          sizeof(header->prefix) + sizeof(header->size) + header->size;
      payload->pending = true;
    }
  }

  if (payload->pending) {
    header = (report_header_t *)(payload->data);
    header->send_time = RTC_Read();
  }

  return payload->pending;
}
