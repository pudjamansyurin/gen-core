/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
//#include "Drivers/_crc.h"
#include "Drivers/_simcom.h"
#include "Libs/_eeprom.h"
#include "Libs/_handlebar.h"
#include "Libs/_reporter.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"
#include "Nodes/HMI1.h"


/* Public functions implementation -------------------------------------------*/
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
  d->req.driver_id = VCU.d.driver_id;
  d->req.events_group = VCU.d.events;
  d->req.vehicle = (int8_t)VCU.d.state;
  d->req.uptime = (VCU.d.uptime * MANAGER_WAKEUP) / 1000;

  // Optional data
  if (frame == FR_FULL) {
    header->size += sizeof(d->opt);

    d->opt.bat = VCU.d.bat / 18;
    d->opt.signal = SIM.signal;
    d->opt.odometer = HBAR.d.trip[HBAR_M_TRIP_ODO] / 1000;

    d->opt.gps.latitude = (int32_t)(VCU.d.gps.latitude * 10000000);
    d->opt.gps.longitude = (int32_t)(VCU.d.gps.longitude * 10000000);
    d->opt.gps.altitude = (uint32_t)VCU.d.gps.altitude;
    d->opt.gps.hdop = (uint8_t)(VCU.d.gps.dop_h * 10);
    d->opt.gps.vdop = (uint8_t)(VCU.d.gps.dop_v * 10);
    d->opt.gps.speed = (uint8_t)VCU.d.gps.speed_kph;
    d->opt.gps.heading = (uint8_t)(VCU.d.gps.heading / 2);
    d->opt.gps.sat_in_use = (uint8_t)VCU.d.gps.sat_in_use;

    d->opt.trip.a = HBAR.d.trip[HBAR_M_TRIP_A];
    d->opt.trip.b = HBAR.d.trip[HBAR_M_TRIP_B];
    d->opt.report.range = HBAR.d.report[HBAR_M_REPORT_RANGE];
    d->opt.report.efficiency = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

    // Debug data
    header->size += sizeof(d->debug);
    memcpy(&(d->debug.motion), &(VCU.d.motion), sizeof(motion_t));

    hmi1_debug_t *hmi1 = &(d->debug.hmi1);
    hmi1->run = HMI1.d.run;

    bms_debug_t *bms = &(d->debug.bms);
    bms->active = BMS.d.active;
    bms->run = BMS.d.run;
    bms->soc= BMS.d.soc;
    bms->fault = BMS.d.fault;
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
      bms->pack[i].id = BMS.d.pack[i].id;
      bms->pack[i].voltage = BMS.d.pack[i].voltage * 100;
      bms->pack[i].current = BMS.d.pack[i].current * 10;
      bms->pack[i].soc = BMS.d.pack[i].soc;
      bms->pack[i].temperature = BMS.d.pack[i].temperature;
    }

    mcu_debug_t *mcu = &(d->debug.mcu);
    mcu->active = MCU.d.active;
    mcu->run = MCU.d.run;
    mcu->rpm = MCU.d.rpm;
    mcu->speed = MCU.RpmToSpeed();
    mcu->reverse = MCU.d.reverse;
    mcu->temperature = (uint16_t)(MCU.d.temperature * 10);
    mcu->drive_mode = MCU.d.drive_mode;
    mcu->torque.commanded = (uint16_t)(MCU.d.torque.commanded * 10);
    mcu->torque.feedback = (uint16_t)(MCU.d.torque.feedback * 10);
    mcu->fault.post = MCU.d.fault.post;
    mcu->fault.run = MCU.d.fault.run;
    mcu->dcbus.current = (uint16_t)(MCU.d.dcbus.current * 10);
    mcu->dcbus.voltage = (uint16_t)(MCU.d.dcbus.voltage * 10);
    mcu->inv.can_mode = MCU.d.inv.can_mode;
    mcu->inv.enabled = MCU.d.inv.enabled;
    mcu->inv.lockout = MCU.d.inv.lockout;
    mcu->inv.discharge = MCU.d.inv.discharge;

    memcpy(&(d->debug.task), &(VCU.d.task), sizeof(rtos_task_t));
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

void RPT_FrameDecider(uint8_t backup, FRAME_TYPE *frame) {
  static uint8_t frameDecider = 0;

  if (backup) {
    *frame = FR_FULL;
    frameDecider = 0;
  } else {
    if (++frameDecider < (RPT_FRAME_FULL / RPT_INTERVAL_NORMAL))
      *frame = FR_SIMPLE;
    else {
      *frame = FR_FULL;
      frameDecider = 0;
    }
  }
}

uint8_t RPT_PayloadPending(payload_t *payload) {
  if (!payload->pending)
    if (osMessageQueueGet(*(payload->pQueue), payload->pPayload, NULL, 0) ==
        osOK)
      payload->pending = 1;

  return payload->pending;
}

uint8_t RPT_WrapPayload(payload_t *payload) {
  report_header_t *header = (report_header_t *)(payload->pPayload);

  header->send_time = RTC_Read();

  //  // Re-calculate CRC
  //  header->crc = CRC_Calculate8(
  //      (uint8_t*) &(header->size),
  //      header->size + sizeof(header->size),
  //      0);

  payload->size = sizeof(header->prefix) +
                  //			sizeof(header->crc) +
                  sizeof(header->size) + header->size;

  return payload->size;
}
