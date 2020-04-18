/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_reporter.h"

/* External variables ----------------------------------------------------------*/
extern db_t DB;

/* Public variables -----------------------------------------------------------*/
report_t REPORT;
response_t RESPONSE;

/* Public functions implementation --------------------------------------------*/
void Reporter_Init(void) {
  // set default data
  LOG_StrLn("Reporter:Init");
  // =============== REPORT ==============
  // header report
  REPORT.header.prefix[0] = PREFIX_REPORT[1];
  REPORT.header.prefix[1] = PREFIX_REPORT[0];
  REPORT.header.crc = 0;
  REPORT.header.size = 0;
  REPORT.header.frame_id = FR_FULL;
  REPORT.header.seq_id = 0;
  EEPROM_UnitID(EE_CMD_R, &(REPORT.header.unit_id));
  // (already set)
  // body required
  REPORT.data.req.rtc_send_datetime = 0;
  REPORT.data.req.rtc_log_datetime = 0;
  REPORT.data.req.driver_id = 1;
  REPORT.data.req.events_group = 0;
  REPORT.data.req.speed = 1;
  // body optional
  REPORT.data.opt.gps.longitude = 0;
  REPORT.data.opt.gps.latitude = 0;
  REPORT.data.opt.gps.hdop = 0;
  REPORT.data.opt.gps.heading = 0;
  EEPROM_Odometer(EE_CMD_R, &(REPORT.data.opt.odometer));
  REPORT.data.opt.bat_voltage = 0;
  REPORT.data.opt.report_range = 0;
  REPORT.data.opt.report_battery = 99;
  REPORT.data.opt.trip_a = 0x01234567;
  REPORT.data.opt.trip_b = 0x89A;

  // =============== RESPONSE ==============
  // header response (copy from report)
  RESPONSE.header = REPORT.header;
  RESPONSE.header.frame_id = FR_RESPONSE;
  // body response
  RESPONSE.data.code = 1;
  strcpy(RESPONSE.data.message, "");
}

void Reporter_SetUnitID(uint32_t unitId) {
  RESPONSE.header.unit_id = unitId;
  REPORT.header.unit_id = unitId;
  EEPROM_UnitID(EE_CMD_W, &unitId);
}

void Reporter_SetOdometer(uint32_t odom) {
  REPORT.data.opt.odometer = odom;
  EEPROM_Odometer(EE_CMD_W, &odom);
}

void Reporter_SetGPS(gps_t *hgps) {
  // parse GPS data
  REPORT.data.opt.gps.latitude = (int32_t) (hgps->latitude * 10000000);
  REPORT.data.opt.gps.longitude = (int32_t) (hgps->longitude * 10000000);
  REPORT.data.opt.gps.hdop = (uint8_t) (hgps->dop_h * 10);
  REPORT.data.opt.gps.heading = (uint8_t) (hgps->heading / 2);
}

void Reporter_SetSpeed(gps_t *hgps) {
  //  static uint16_t odom_meter = 0;
  // FIXME use real speed calculation
  // calculate speed from GPS data
  REPORT.data.req.speed = hgps->speed_kph;
  // dummy odometer
  if (hgps->speed_mps > 10) {
    Reporter_SetOdometer(REPORT.data.opt.odometer + (hgps->speed_mps * DB.bms.interval));
  }
  //  odom_meter += (hgps->speed_mps * REPORT_INTERVAL_SIMPLE);
  //  if (odom_meter >= 1000) {
  //    Reporter_SetOdometer(REPORT.data.opt.odometer++);
  //    odom_meter = 0;
  //  }
}

void Reporter_SetEvents(uint64_t value) {
  REPORT.data.req.events_group = value;
}

void Reporter_WriteEvent(uint64_t event_id, uint8_t value) {
  if (value & 1) {
    BV(REPORT.data.req.events_group, _BitPosition(event_id));
  } else {
    BC(REPORT.data.req.events_group, _BitPosition(event_id));
  }
}

uint8_t Reporter_ReadEvent(uint64_t event_id) {
  return BSR((REPORT.data.req.events_group & event_id), _BitPosition(event_id));
}

void Reporter_Capture(FRAME_TYPE frame) {
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
      // set battery voltage
      REPORT.data.opt.bat_voltage = DB.vcu.battery / 18;
    }
    // CRC will be recalculated when sending the payload
    // (because RTC_Send_Datetime will be changed later)
    REPORT.header.crc = 0;
  }
}

