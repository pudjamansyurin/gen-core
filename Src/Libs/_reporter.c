/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"

// variable list
report_t REPORT;
response_t RESPONSE;

// function list
void Reporter_Reset(frame_type frame) {
  // set default data
  // header report
  REPORT.header.prefix = FRAME_PREFIX;
  REPORT.header.crc = 0;
  REPORT.header.size = 0;
  REPORT.header.frame_id = frame;
  REPORT.header.seq_id = 0;
  Reporter_Set_UnitID(REPORT_UNITID);

  if (frame == FR_RESPONSE) {
    // header response
    // (copy from report)
    RESPONSE.header = REPORT.header;

    // body response
    RESPONSE.data.code = 1;
    strcpy(RESPONSE.data.message, "");

  } else {
    // header report
    // (already set)

    // body required
    if (frame == FR_SIMPLE || frame == FR_FULL) {
      REPORT.data.req.rtc_send_datetime = 0;
      REPORT.data.req.rtc_log_datetime = 0;
      REPORT.data.req.driver_id = 1;
      REPORT.data.req.events_group = 0;
      REPORT.data.req.speed = 1;
    }

    // body optional
    if (frame == FR_FULL) {
      // FIXME: default value should be zero
      REPORT.data.opt.gps.longitude = 112.6935779 * 10000000;
      REPORT.data.opt.gps.latitude = -7.4337599 * 10000000;
      REPORT.data.opt.gps.hdop = 255;
      REPORT.data.opt.gps.heading = 0;
      REPORT.data.opt.odometer = EEPROM_Get_Odometer();
      REPORT.data.opt.bat_voltage = 2600 / 13;
      REPORT.data.opt.report_range = 0;
      REPORT.data.opt.report_battery = 99;
      REPORT.data.opt.trip_a = 0x01234567;
      REPORT.data.opt.trip_b = 0x89A;
    }
  }
}

void Reporter_Set_UnitID(uint32_t unitId) {
  RESPONSE.header.unit_id = unitId;
  REPORT.header.unit_id = unitId;
}

void Reporter_Set_Odometer(uint32_t odom) {
  REPORT.data.opt.odometer = odom;
  EEPROM_Save_Odometer(odom);
}

void Reporter_Set_GPS(gps_t *hgps) {
  // parse GPS data
  REPORT.data.opt.gps.latitude = (int32_t) (hgps->latitude * 10000000);
  REPORT.data.opt.gps.longitude = (int32_t) (hgps->longitude * 10000000);
  REPORT.data.opt.gps.hdop = (uint8_t) (hgps->dop_h * 10);
  REPORT.data.opt.gps.heading = (uint8_t) (hgps->heading / 2);
}

void Reporter_Set_Speed(gps_t *hgps) {
  // FIXME use real speed calculation
  // calculate speed from GPS data
  REPORT.data.req.speed = hgps->speed_kph;
  // save ODOMETER to flash (non-volatile)
  Reporter_Set_Odometer(EEPROM_Get_Odometer() + (hgps->speed_mps * REPORT_INTERVAL_SIMPLE));

}

void Reporter_Set_Events(uint64_t value) {
  REPORT.data.req.events_group = value;
}

void Reporter_Set_Event(uint64_t event_id, uint8_t value) {
  if (value & 1) {
    // set
    SetBitOf(REPORT.data.req.events_group, _BitPosition(event_id));
  } else {
    // clear
    ClearBitOf(REPORT.data.req.events_group, _BitPosition(event_id));
  }
}

uint8_t Reporter_Read_Event(uint64_t event_id) {
  return (REPORT.data.req.events_group & event_id) >> _BitPosition(event_id);
}

void Reporter_Capture(frame_type frame) {
  if (frame == FR_RESPONSE) {
    //Reconstruct the header
    RESPONSE.header.seq_id++;
    RESPONSE.header.frame_id = FR_RESPONSE;
    RESPONSE.header.size = sizeof(RESPONSE.header.frame_id) +
        sizeof(RESPONSE.header.unit_id) +
        sizeof(RESPONSE.header.seq_id) +
        sizeof(RESPONSE.data.code) +
        strlen(RESPONSE.data.message);
    RESPONSE.header.crc = CRC_Calculate8(
        (uint8_t*) &(RESPONSE.header.size),
        RESPONSE.header.size + sizeof(RESPONSE.header.size), 1);

  } else {
    // Reconstruct the body
    REPORT.data.req.rtc_log_datetime = RTC_Read();
    REPORT.data.req.rtc_send_datetime = 0;

    // Reconstruct the header
    REPORT.header.seq_id++;
    REPORT.header.frame_id = frame;
    REPORT.header.size = sizeof(REPORT.header.frame_id) +
        sizeof(REPORT.header.unit_id) +
        sizeof(REPORT.header.seq_id) +
        sizeof(REPORT.data.req);
    // Add opt on FR_FULL
    if (frame == FR_FULL) {
      REPORT.header.size += sizeof(REPORT.data.opt);
    }
    // CRC will be recalculated when sending the payload
    // (because RTC_Send_Datetime will be changed later)
    REPORT.header.crc = 0;
  }
}

