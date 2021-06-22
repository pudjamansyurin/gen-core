/*
 * _debugger.h
 *
 *  Created on: Apr 28, 2021
 *      Author: GEN
 */

#ifndef INC_LIBS__DEBUGGER_H_
#define INC_LIBS__DEBUGGER_H_

/* Includes
 * --------------------------------------------*/
#include "App/_task.h"
#include "Libs/_utils.h"
#include "Nodes/MCU.h"

/* Exported structs
 * --------------------------------------------*/
typedef struct __attribute__((packed)) {
  int8_t state;
  uint16_t events;
  uint8_t buffered;
  uint8_t battery;
  uint32_t uptime;
} vcu_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t reverse;
  struct __attribute__((packed)) {
    uint8_t drive;
    uint8_t trip;
    uint8_t report;
  } mode;
  struct __attribute__((packed)) {
    uint16_t a;
    uint16_t b;
    uint16_t odometer;
  } trip;
  struct __attribute__((packed)) {
    uint8_t range;
    uint8_t efficiency;
  } report;
} hbar_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t signal;
  int8_t state;
  int8_t ipstatus;
} net_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
  uint8_t sat_in_use;
  uint8_t hdop;
  uint8_t vdop;
  uint8_t speed;
  uint8_t heading;
  int32_t longitude;
  int32_t latitude;
  uint16_t altitude;
} gps_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
  uint8_t det_active;
  struct __attribute__((packed)) {
    int16_t x;
    int16_t y;
    int16_t z;
  } accel;
  struct __attribute__((packed)) {
    int16_t x;
    int16_t y;
    int16_t z;
  } gyro;
  struct __attribute__((packed)) {
    int16_t pitch;
    int16_t roll;
  } tilt;
  struct __attribute__((packed)) {
    uint16_t accel;
    uint16_t gyro;
    uint16_t tilt;
    uint16_t temp;
  } total;
} mems_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
  uint8_t nearby;
} remote_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t verified;
  uint8_t driver_id;
} finger_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
  uint8_t mute;
  uint8_t volume;
} audio_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
} hmi1_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
  uint8_t run;
  uint8_t soc;
  uint16_t fault;
  struct __attribute__((packed)) {
    uint32_t id;
    uint16_t fault;
    uint16_t voltage;
    uint16_t current;
    uint8_t soc;
    uint16_t temperature;
  } packs[2];
} bms_dbg_t;

typedef struct __attribute__((packed)) {
  uint8_t active;
  uint8_t run;
  uint8_t reverse;
  uint8_t drive_mode;
  uint8_t speed;
  int16_t rpm;
  int16_t temperature;
  struct {
    uint32_t post;
    uint32_t run;
  } fault;
  struct {
    int16_t commanded;
    int16_t feedback;
  } torque;
  struct {
    int16_t current;
    int16_t voltage;
  } dcbus;
  struct {
    uint8_t enabled;
    uint8_t lockout;
    uint8_t discharge;
  } inv;
  struct __attribute__((packed)) {
    int16_t rpm_max;
    uint8_t speed_max;
    mcu_template_t tpl[HBAR_M_DRIVE_MAX];
  } par;
} mcu_dbg_t;

typedef struct __attribute__((packed)) {
  tasks_stack_t stack;
  tasks_wakeup_t wakeup;
} tasks_dbg_t;

/* Public functions prototype
 * --------------------------------------------*/
void DBG_GetVCU(vcu_dbg_t *vcu);
void DBG_GetNET(net_dbg_t *net);
void DBG_GetGPS(gps_dbg_t *gps);
void DBG_GetMEMS(mems_dbg_t *mems);
void DBG_GetRMT(remote_dbg_t *rmt);
void DBG_GetFGR(finger_dbg_t *fgr);
void DBG_GetAudio(audio_dbg_t *audio);
void DBG_GetHBAR(hbar_dbg_t *hbar);
void DBG_GetHMI1(hmi1_dbg_t *hmi1);
void DBG_GetBMS(bms_dbg_t *bms);
void DBG_GetMCU(mcu_dbg_t *mcu);
void DBG_GetTasks(tasks_dbg_t *tasks);

#endif /* INC_LIBS__DEBUGGER_H_ */
