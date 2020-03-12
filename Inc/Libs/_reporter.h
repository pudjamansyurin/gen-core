/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"
#include "_eeprom.h"
#include "_gps.h"
#include "_rtc.h"
#include "_crc.h"

/* Exported enum ---------------------------------------------------------------*/
typedef enum {
  FR_RESPONSE = 0,
  FR_SIMPLE = 1,
  FR_FULL = 2,
} frame_type;

/* Exported struct --------------------------------------------------------------*/
// header frame (for report & response)
typedef struct __attribute__((packed)) {
  uint16_t prefix;
  uint32_t crc;
  uint8_t size;
  uint8_t frame_id;
  uint32_t unit_id;
  uint16_t seq_id;
} report_header_t;

// report frame
typedef struct __attribute__((packed)) {
  struct __attribute__((packed)) {
    uint64_t rtc_send_datetime;
    uint64_t rtc_log_datetime;
    uint8_t driver_id;
    uint64_t events_group;
    uint8_t speed;
  } req;
  struct __attribute__((packed)) {
    struct __attribute__((packed)) {
      int32_t longitude;
      int32_t latitude;
      uint8_t hdop;
      uint8_t heading;
    } gps;
    uint32_t odometer;
    uint8_t bat_voltage;
    uint8_t report_range;
    uint8_t report_battery;
    uint32_t trip_a;
    uint32_t trip_b;
  } opt;
} report_data_t;

typedef struct __attribute__((packed)) {
  report_header_t header;
  report_data_t data;
} report_t;

// response frame
typedef struct __attribute__((packed)) {
  report_header_t header;
  struct __attribute__((packed)) {
    uint8_t code;
    char message[50];
  } data;
} response_t;

// command frame (from server)
typedef struct __attribute__((packed)) {
  struct __attribute__((packed)) {
    uint16_t prefix;
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
typedef struct __attribute__((packed)) {
  uint16_t prefix;
  uint8_t frame_id;
  uint16_t seq_id;
} ack_t;

/* Public functions prototype ------------------------------------------------*/
void Reporter_Reset(frame_type frame);
void Reporter_SetUnitID(uint32_t unitId);
void Reporter_SetOdometer(uint32_t odom);
void Reporter_SetGPS(gps_t *hgps);
void Reporter_SetSpeed(gps_t *hgps);
void Reporter_SetEvents(uint64_t value);
void Reporter_WriteEvent(uint64_t event_id, uint8_t value);
void Reporter_Capture(frame_type frame);
uint8_t Reporter_ReadEvent(uint64_t event_id);

#endif /* REPORTER_H_ */
