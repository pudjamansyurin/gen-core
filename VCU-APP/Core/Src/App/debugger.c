/*
 * debugger.c
 *
 *  Created on: Apr 28, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/debugger.h"

#include "App/vehicle.h"
#include "Drivers/bat.h"
#include "Drivers/simcom.h"
#include "Libs/audio.h"
#include "Libs/eeprom.h"
#include "Libs/finger.h"
#include "Libs/hbar.h"
#include "Libs/remote.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"

/* Public functions implementation
 * --------------------------------------------*/
void DBG_GetVCU(vcu_dbg_t *vcu) {
  vcu->events = EVT_Val();
  vcu->state = (int8_t)VHC_IO_State();
  vcu->uptime = (VCU.d.uptime * MANAGER_WAKEUP_MS) / 1000;
  vcu->buffered = VCU.d.buffered;
  vcu->battery = BAT_IO_Voltage() / 18;
}

void DBG_GetEEPROM(ee_dbg_t *ee) {
  ee->active = EE_IO_Active();
  ee->used = EE_IO_Used();
}

void DBG_GetHBAR(hbar_dbg_t *hbar) {
  hbar->reverse = HB_IO_Pin(HBP_REVERSE);
  hbar->mode.drive = HB_IO_Sub(HBM_DRIVE);
  hbar->mode.trip = HB_IO_Sub(HBM_TRIP);
  hbar->mode.avg = HB_IO_Sub(HBM_AVG);
  hbar->trip.a = HB_IO_Trip(HBMS_TRIP_A);
  hbar->trip.b = HB_IO_Trip(HBMS_TRIP_B);
  hbar->trip.odometer = HB_IO_Trip(HBMS_TRIP_ODO);
  hbar->avg.range = HB_IO_Average(HBMS_AVG_RANGE);
  hbar->avg.efficiency = HB_IO_Average(HBMS_AVG_EFFICIENCY);
}

void DBG_GetNET(net_dbg_t *net) {
  net->signal = SIMSta_IO_Signal();
  net->state = SIMSta_IO_State();
  net->ipstatus = SIMSta_IO_Ip();
}

void DBG_GetGPS(gps_dbg_t *gps) {
  gps->active = GPS_IO_Data()->active;
  gps->sat_in_use = (uint8_t)GPS_IO_Nmea()->sats_in_use;
  gps->hdop = (uint8_t)(GPS_IO_Nmea()->dop_h * 10);
  gps->vdop = (uint8_t)(GPS_IO_Nmea()->dop_v * 10);
  gps->speed = (uint8_t)nmea_to_speed(GPS_IO_Nmea()->speed, nmea_speed_kph);
  gps->heading = (uint8_t)(GPS_IO_Nmea()->coarse / 2);
  gps->latitude = (int32_t)(GPS_IO_Nmea()->latitude * 10000000);
  gps->longitude = (int32_t)(GPS_IO_Nmea()->longitude * 10000000);
  gps->altitude = (uint16_t)GPS_IO_Nmea()->altitude;
}

void DBG_GetMEMS(mems_dbg_t *mems) {
  mems->active = MEMS_IO_Active();
  mems->motion_active = MEMS_IO_MotionActive();
  mems->accel.x = MEMS_IO_Raw()->accel.x * 100;
  mems->accel.y = MEMS_IO_Raw()->accel.y * 100;
  mems->accel.z = MEMS_IO_Raw()->accel.z * 100;

  mems->gyro.x = MEMS_IO_Raw()->gyro.x * 10;
  mems->gyro.y = MEMS_IO_Raw()->gyro.y * 10;
  mems->gyro.z = MEMS_IO_Raw()->gyro.z * 10;

  mems->tilt.pitch = MEMS_IO_Tilt(MTILT_NOW)->pitch * 10;
  mems->tilt.roll = MEMS_IO_Tilt(MTILT_NOW)->roll * 10;

  mems->total.accel = MEMS_IO_Total()->accel * 100;
  mems->total.gyro = MEMS_IO_Total()->gyro * 10;
  mems->total.tilt = MEMS_IO_Total()->tilt * 10;
  mems->total.temp = MEMS_IO_Raw()->temp * 10;
}

void DBG_GetRMT(remote_dbg_t *rmt) {
  rmt->active = RMT_IO_Active();
  rmt->nearby = RMT_IO_Nearby();
}

void DBG_GetFGR(finger_dbg_t *fgr) {
  fgr->verified = FGR_IO_Data()->verified;
  fgr->driver_id = FGR_IO_Data()->id;
}

void DBG_GetAudio(audio_dbg_t *audio) {
  audio->active = AUDIO_IO_Data()->active;
  audio->mute = AUDIO_IO_Data()->mute;
  audio->volume = AUDIO_IO_Data()->volume;
}

void DBG_GetHMI1(hmi1_dbg_t *hmi1) { hmi1->active = HMI1.d.active; }

void DBG_GetBMS(bms_dbg_t *bms) {
  bms->active = BMS.d.active;
  bms->run = BMS.d.run;
  bms->fault = BMS.d.fault;
  bms->soc = BMS.d.soc;
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    bms_pack_t *p = &(BMS.packs[i]);

    bms->packs[i].id = p->id;
    bms->packs[i].fault = p->fault;
    bms->packs[i].voltage = p->voltage * 100;
    bms->packs[i].current = p->current * 10;
    bms->packs[i].soc = p->soc;
    bms->packs[i].temperature = p->temperature;
  }
}

void DBG_GetMCU(mcu_dbg_t *mcu) {
  mcu->active = MCU.d.active;
  mcu->run = MCU.d.run;
  mcu->rpm = MCU.d.rpm;
  mcu->speed = MCU_RpmToSpeed(MCU.d.rpm);
  mcu->reverse = MCU.d.reverse;
  mcu->temperature = (uint16_t)(MCU.d.temperature * 10);
  mcu->drive_mode = MCU.d.drive_mode;
  mcu->torque.commanded = (uint16_t)(MCU.d.torque.commanded * 10);
  mcu->torque.feedback = (uint16_t)(MCU.d.torque.feedback * 10);
  mcu->fault.post = MCU.d.fault.post;
  mcu->fault.run = MCU.d.fault.run;
  mcu->dcbus.current = (uint16_t)(MCU.d.dcbus.current * 10);
  mcu->dcbus.voltage = (uint16_t)(MCU.d.dcbus.voltage * 10);
  mcu->inv.enabled = MCU.d.inv.enabled;
  mcu->inv.lockout = MCU.d.inv.lockout;
  mcu->inv.discharge = MCU.d.inv.discharge;

  mcu->par.rpm_max = MCU.d.par.rpm_max;
  mcu->par.speed_max = MCU_RpmToSpeed(MCU.d.par.rpm_max);
  for (uint8_t m = 0; m < HBMS_DRIVE_MAX; m++) {
    mcu->par.tpl[m].discur_max = MCU.d.par.tpl[m].discur_max;
    mcu->par.tpl[m].torque_max = MCU.d.par.tpl[m].torque_max * 10;
  }
}

void DBG_GetTasks(tasks_dbg_t *tasks) {
  for (uint8_t task = 0; task < TASK_MAX; task++) {
    tasks->stack[task] = TASK_IO_Stack(task);
    tasks->wakeup[task] = TASK_IO_Wakeup(task);
  }
}
