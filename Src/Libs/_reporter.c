/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"
#include "_crc.h"

// variable list
frame_t FR;

// function list
void Reporter_Reset(frame_type frame) {
  // set default data
  // header report
  FR.report.header.prefix = FRAME_PREFIX;
  FR.report.header.crc = 0;
  FR.report.header.size = 0;
  FR.report.header.frame_id = frame;
  FR.report.header.seq_id = 0;
  Reporter_Set_UnitID(REPORT_UNITID);

  if (frame == FR_RESPONSE) {
    // header response
    // (copy from report)
    FR.response.header = FR.report.header;

    // body response
    FR.response.data.code = 1;
    strcpy(FR.response.data.message, "");

  } else {
    // header report
    // (already set)

    // body required
    if (frame == FR_SIMPLE || frame == FR_FULL) {
      FR.report.data.req.rtc_send_datetime = 0;
      FR.report.data.req.rtc_log_datetime = 0;
      FR.report.data.req.driver_id = 1;
      FR.report.data.req.events_group = 0;
      FR.report.data.req.speed = 1;
    }

    // body optional
    if (frame == FR_FULL) {
      // FIXME: default value should be zero
      FR.report.data.opt.gps.longitude = 112.6935779 * 10000000;
      FR.report.data.opt.gps.latitude = -7.4337599 * 10000000;
      FR.report.data.opt.gps.hdop = 255;
      FR.report.data.opt.gps.heading = 0;
      FR.report.data.opt.odometer = EEPROM_Get_Odometer();
      FR.report.data.opt.bat_voltage = 2600 / 13;
      FR.report.data.opt.report_range = 0;
      FR.report.data.opt.report_battery = 99;
      FR.report.data.opt.trip_a = 0x01234567;
      FR.report.data.opt.trip_b = 0x89A;
    }
  }
}

void Reporter_Set_UnitID(uint32_t unitId) {
  FR.response.header.unit_id = unitId;
  FR.report.header.unit_id = unitId;
}

void Reporter_Set_Odometer(uint32_t odom) {
  FR.report.data.opt.odometer = odom;
  EEPROM_Save_Odometer(odom);
}

void Reporter_Set_GPS(gps_t *hgps) {
  // parse GPS data
  FR.report.data.opt.gps.latitude = (int32_t) (hgps->latitude * 10000000);
  FR.report.data.opt.gps.longitude = (int32_t) (hgps->longitude * 10000000);
  FR.report.data.opt.gps.hdop = (uint8_t) (hgps->dop_h * 10);
  FR.report.data.opt.gps.heading = (uint8_t) (hgps->heading / 2);
}

void Reporter_Set_Speed(gps_t *hgps) {
  // FIXME use real speed calculation
  // calculate speed from GPS data
  FR.report.data.req.speed = hgps->speed_kph;
  // save ODOMETER to flash (non-volatile)
  Reporter_Set_Odometer(EEPROM_Get_Odometer() + (hgps->speed_mps * REPORT_INTERVAL_SIMPLE));

}

void Reporter_Set_Events(uint64_t value) {
  FR.report.data.req.events_group = value;
}

void Reporter_Set_Event(uint64_t event_id, uint8_t bool) {
  if (bool & 1) {
    // set
    SetBitOf(FR.report.data.req.events_group, BSP_BitPosition(event_id));
  } else {
    // clear
    ClearBitOf(FR.report.data.req.events_group, BSP_BitPosition(event_id));
  }
}

uint8_t Reporter_Read_Event(uint64_t event_id) {
  return (FR.report.data.req.events_group & event_id) >> BSP_BitPosition(event_id);
}

void Reporter_Capture(frame_type frame) {
  if (frame == FR_RESPONSE) {
    //Reconstruct the header
    FR.response.header.seq_id++;
    FR.response.header.frame_id = FR_RESPONSE;
    FR.response.header.size = sizeof(FR.response.header.frame_id) +
        sizeof(FR.response.header.unit_id) +
        sizeof(FR.response.header.seq_id) +
        sizeof(FR.response.data.code) +
        strlen(FR.response.data.message);
    FR.response.header.crc = CRC_Calculate8(
        (uint8_t*) &(FR.response.header.size),
        FR.response.header.size + sizeof(FR.response.header.size), 1);

  } else {
    // Reconstruct the body
    FR.report.data.req.rtc_log_datetime = RTC_Read();
    FR.report.data.req.rtc_send_datetime = 0;

    // Reconstruct the header
    FR.report.header.seq_id++;
    FR.report.header.frame_id = frame;
    FR.report.header.size = sizeof(FR.report.header.frame_id) +
        sizeof(FR.report.header.unit_id) +
        sizeof(FR.report.header.seq_id) +
        sizeof(FR.report.data.req);
    // Add opt on FR_FULL
    if (frame == FR_FULL) {
      FR.report.header.size += sizeof(FR.report.data.opt);
    }
    // CRC will be recalculated when sending the payload
    // (because RTC_Send_Datetime will be changed later)
    FR.report.header.crc = 0;
  }
}

