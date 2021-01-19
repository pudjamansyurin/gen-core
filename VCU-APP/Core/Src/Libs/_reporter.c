/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Drivers/_crc.h"
#include "Drivers/_simcom.h"
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"
#include "Libs/_eeprom.h"
#include "Nodes/BMS.h"

/* Private functions declarations -------------------------------------------*/
static void RPT_ReportCaptureHeader(FRAME_TYPE frame, report_t *report, uint32_t unit_id);
static void Report_SetCRC(report_t *report);
static void Response_SetCRC(response_t *response);

/* Public functions implementation -------------------------------------------*/
void RPT_ReportInit(FRAME_TYPE frame, report_t *report, uint16_t *seq_id_report) {
  printf("Reporter:ReportInit\n");

  report->header.prefix[0] = PREFIX_REPORT[1];
  report->header.prefix[1] = PREFIX_REPORT[0];
  report->header.seq_id = *seq_id_report;
}

void RPT_ResponseInit(response_t *response, uint16_t *seq_id_response) {
  printf("Reporter:ResponseInit\n");

  response->header.prefix[0] = PREFIX_REPORT[1];
  response->header.prefix[1] = PREFIX_REPORT[0];
  response->header.seq_id = *seq_id_response;
}

void RPT_ReportCapture(FRAME_TYPE frame, report_t *report, vcu_data_t *vcu, bms_data_t *bms, hbar_data_t *hbar) {
  RPT_ReportCaptureHeader(frame, report, vcu->unit_id);

  // Reconstruct the body
  report->data.req.vcu.driver_id = vcu->driver_id;
  report->data.req.vcu.events_group = vcu->events;
  report->data.req.vcu.rtc.log = RTC_Read();

  for (uint8_t i = 0; i < BMS_COUNT ; i++) {
    report->data.req.bms.pack[i].id = bms->pack[i].id;
    report->data.req.bms.pack[i].voltage = bms->pack[i].voltage * 100;
    report->data.req.bms.pack[i].current = (bms->pack[i].current + 50) * 100;
  }

  // Add more (if full frame)
  if (frame == FR_FULL) {
    report->data.opt.vcu.vehicle = (int8_t) vcu->state.vehicle;

    report->data.opt.vcu.gps.latitude = (int32_t) (vcu->gps.latitude * 10000000);
    report->data.opt.vcu.gps.longitude = (int32_t) (vcu->gps.longitude * 10000000);
    report->data.opt.vcu.gps.altitude = (uint32_t) vcu->gps.altitude;
    report->data.opt.vcu.gps.hdop = (uint8_t) (vcu->gps.dop_h * 10);
    report->data.opt.vcu.gps.vdop = (uint8_t) (vcu->gps.dop_v * 10);
    report->data.opt.vcu.gps.heading = (uint8_t) (vcu->gps.heading / 2);
    report->data.opt.vcu.gps.sat_in_use = (uint8_t) vcu->gps.sat_in_use;

    report->data.opt.vcu.speed = vcu->speed;
    report->data.opt.vcu.odometer = vcu->odometer;

    report->data.opt.vcu.trip.a = hbar->trip[HBAR_M_TRIP_A];
    report->data.opt.vcu.trip.b = hbar->trip[HBAR_M_TRIP_B];
    report->data.opt.vcu.report.range = hbar->report[HBAR_M_REPORT_RANGE];
    report->data.opt.vcu.report.efficiency = hbar->report[HBAR_M_REPORT_AVERAGE];

    report->data.opt.vcu.signal = SIM.signal;
    report->data.opt.vcu.bat = vcu->bat / 18;

    for (uint8_t i = 0; i < BMS_COUNT ; i++) {
      report->data.opt.bms.pack[i].soc = bms->pack[i].soc * 100;
      report->data.opt.bms.pack[i].temperature = (bms->pack[i].temperature + 40) * 10;
    }

    // debug data
    memcpy(&(report->data.test.motion), &(vcu->motion), sizeof(motion_t));
    memcpy(&(report->data.test.task), &(vcu->task), sizeof(rtos_task_t));
  }
}

void RPT_ResponseCapture(response_t *response, uint32_t *unit_id) {
  //Reconstruct the header
  response->header.seq_id++;
  response->header.unit_id = *unit_id;
  response->header.frame_id = FR_RESPONSE;
  response->header.size = sizeof(response->header.frame_id) +
      sizeof(response->header.unit_id) +
      sizeof(response->header.seq_id) +
      sizeof(response->data.code) +
      strlen(response->data.message);
}

void RPT_CommandDebugger(command_t *cmd) {
  printf("Command:Payload [%u-%u] = %.*s\n",
      cmd->data.code,
      cmd->data.sub_code,
      sizeof(cmd->data.value),
      (char*) &(cmd->data.value)
  );
}

FRAME_TYPE RPT_FrameDecider(uint8_t backup) {
  static uint8_t frameDecider = 0;
  FRAME_TYPE frame;

  if (backup) {
    frame = FR_FULL;
    frameDecider = 0;
  } else {
    if (++frameDecider < (RPT_FRAME_FULL / RPT_INTERVAL_NORMAL ))
      frame = FR_SIMPLE;
    else {
      frame = FR_FULL;
      frameDecider = 0;
    }
  }

  return frame;
}

uint8_t RPT_PayloadPending(payload_t *payload) {
  uint32_t notif;

  // Handle Full Buffer
  if (payload->type == PAYLOAD_REPORT)
    if (_osThreadFlagsWait(&notif, EVT_IOT_DISCARD, osFlagsWaitAny, 0))
      payload->pending = 0;

  // Check logs
  if (!payload->pending)
    if (osMessageQueueGet(*(payload->pQueue), payload->pPayload, NULL, 0) == osOK)
      payload->pending = 1;

  return payload->pending;
}

uint8_t RPT_WrapPayload(payload_t *payload) {
  header_t *pHeader = (header_t*) (payload->pPayload);

  // Re-calculate CRC
  if (payload->type == PAYLOAD_REPORT)
    Report_SetCRC((report_t*) payload->pPayload);
  else
    Response_SetCRC((response_t*) payload->pPayload);

  // Calculate final size
  return sizeof(pHeader->prefix)
              + sizeof(pHeader->crc)
              + sizeof(pHeader->size)
              + pHeader->size;
}

void RPT_FinishedPayload(payload_t *payload) {
  header_t *pHeader = (header_t*) (payload->pPayload);

  EEPROM_SequentialID(EE_CMD_W, pHeader->seq_id, payload->type);
  payload->pending = 0;
}

/* Private functions implementation -------------------------------------------*/
static void RPT_ReportCaptureHeader(FRAME_TYPE frame, report_t *report, uint32_t unit_id) {
  report->header.seq_id++;
  report->header.unit_id = unit_id;
  report->header.frame_id = frame;
  report->header.size = sizeof(report->header.frame_id) +
      sizeof(report->header.unit_id) +
      sizeof(report->header.seq_id) +
      sizeof(report->data.req);

  if (frame == FR_FULL)
    report->header.size += sizeof(report->data.opt) + sizeof(report->data.test);
}

static void Report_SetCRC(report_t *report) {
  // get current sending date-time
  report->data.req.vcu.rtc.send = RTC_Read();
  // recalculate the CRC
  report->header.crc = CRC_Calculate8(
      (uint8_t*) &(report->header.size),
      report->header.size + sizeof(report->header.size),
      0);
}

static void Response_SetCRC(response_t *response) {
  response->header.crc = CRC_Calculate8(
      (uint8_t*) &(response->header.size),
      response->header.size + sizeof(response->header.size),
      0);
}
