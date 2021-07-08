/*
 * _debugger.c
 *
 *  Created on: Apr 28, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_debugger.h"

#include "Drivers/_bat.h"
#include "Drivers/_simcom.h"
#include "Libs/_audio.h"
#include "Libs/_eeprom.h"
#include "Libs/_finger.h"
#include "Libs/_hbar.h"
#include "Libs/_remote.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"

/* Public functions implementation
 * --------------------------------------------*/
void DBG_GetVCU(vcu_dbg_t *vcu) {
  vcu->events = EVT_Val();
  vcu->state = (int8_t)VCU.d.state;
  vcu->uptime = (VCU.d.uptime * MANAGER_WAKEUP_MS) / 1000;
  vcu->buffered = VCU.d.buffered;
  vcu->battery = BAT_ScanValue() / 18;
}

void DBG_GetEEPROM(ee_dbg_t *ee) {
  ee->active = EEPROM.active;
  ee->used = EEPROM.used;
}

void DBG_GetHBAR(hbar_dbg_t *hbar) {
  hbar->reverse = HB_IO_GetPin(HBP_REVERSE);
  hbar->mode.drive = HB_IO_GetSub(HBM_DRIVE);
  hbar->mode.trip = HB_IO_GetSub(HBM_TRIP);
  hbar->mode.avg = HB_IO_GetSub(HBM_AVG);
  hbar->trip.a = HB_IO_GetTrip(HBMS_TRIP_A);
  hbar->trip.b = HB_IO_GetTrip(HBMS_TRIP_B);
  hbar->trip.odometer = HB_IO_GetTrip(HBMS_TRIP_ODO);
  hbar->avg.range = HB_IO_GetAverage(HBMS_AVG_RANGE);
  hbar->avg.efficiency = HB_IO_GetAverage(HBMS_AVG_EFFICIENCY);
}

void DBG_GetNET(net_dbg_t *net) {
  net->signal = SIM.d.signal;
  net->state = SIM.d.state;
  net->ipstatus = SIM.d.ipstatus;
}

void DBG_GetGPS(gps_dbg_t *gps) {
  const gps_data_t* d = GPS_IO_GetData();

  gps->active = d->active;
  gps->sat_in_use = (uint8_t)d->nmea.sats_in_use;
  gps->hdop = (uint8_t)(d->nmea.dop_h * 10);
  gps->vdop = (uint8_t)(d->nmea.dop_v * 10);
  gps->speed = (uint8_t)nmea_to_speed(d->nmea.speed, nmea_speed_kph);
  gps->heading = (uint8_t)(d->nmea.coarse / 2);
  gps->latitude = (int32_t)(d->nmea.latitude * 10000000);
  gps->longitude = (int32_t)(d->nmea.longitude * 10000000);
  gps->altitude = (uint16_t)d->nmea.altitude;
}

void DBG_GetMEMS(mems_dbg_t *mems) {
	const mems_raw_t* raw = MEMS_IO_GetRaw();
	const mems_total_t* total = MEMS_IO_GetTotal();
	const mems_tilt_t* tilt = MEMS_IO_GetTilt(MTILT_NOW);

  mems->active = MEMS_IO_GetActive();
  mems->motion_active = MEMS_IO_GetMotionActive();
  mems->accel.x = raw->accel.x * 100;
  mems->accel.y = raw->accel.y * 100;
  mems->accel.z = raw->accel.z * 100;

  mems->gyro.x = raw->gyro.x * 10;
  mems->gyro.y = raw->gyro.y * 10;
  mems->gyro.z = raw->gyro.z * 10;

  mems->tilt.pitch = tilt->pitch * 10;
  mems->tilt.roll = tilt->roll * 10;

  mems->total.accel = total->accel * 100;
  mems->total.gyro = total->gyro * 10;
  mems->total.tilt = total->tilt * 10;
  mems->total.temp = raw->temp * 10;
}

void DBG_GetRMT(remote_dbg_t *rmt) {
  rmt->active = RMT.d.active;
  rmt->nearby = RMT.d.nearby;
}

void DBG_GetFGR(finger_dbg_t *fgr) {
  const finger_data_t* finger = FGR_IO_GetData();

  fgr->verified = finger->verified;
  fgr->driver_id = finger->id;
}

void DBG_GetAudio(audio_dbg_t *audio) {
  audio_data_t d = AUDIO_GetData();
  audio->active = d.active;
  audio->mute = d.mute;
  audio->volume = d.volume;
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
    tasks->stack[task] = TASK_IO_GetStack(task);
    tasks->wakeup[task] = TASK_IO_GetWakeup(task);
  }
}
