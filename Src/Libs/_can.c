/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_can.h"
#include "_reporter.h"

/* External variables ----------------------------------------------------------*/
extern canbus_t CB;

/* Public functions implementation --------------------------------------------*/
uint8_t CANT_VCU_Switch(db_t *db, sw_t *sw) {
  static TickType_t tick, tickSein;
  static uint8_t iSeinLeft = 0, iSeinRight = 0;
  static status_t iStatus;

  // indicator manipulator
  if ((osKernelGetTickCount() - tick) >= pdMS_TO_TICKS(500)) {
    // finger
    if (db->hmi1.status.finger) {
      tick = osKernelGetTickCount();
      iStatus.finger = !iStatus.finger;
    }
    // keyless
    if (db->hmi1.status.keyless) {
      tick = osKernelGetTickCount();
      iStatus.keyless = !iStatus.keyless;
    }
    // temperature
    if (db->hmi1.status.temperature) {
      tick = osKernelGetTickCount();
      iStatus.temperature = !iStatus.temperature;
    }
  }

  // sein manipulator
  if ((osKernelGetTickCount() - tickSein) >= pdMS_TO_TICKS(500)) {
    if (sw->list[SW_K_SEIN_LEFT].state && sw->list[SW_K_SEIN_RIGHT].state) {
      // hazard
      tickSein = osKernelGetTickCount();
      iSeinLeft = !iSeinLeft;
      iSeinRight = iSeinLeft;
    } else if (sw->list[SW_K_SEIN_LEFT].state) {
      // left sein
      tickSein = osKernelGetTickCount();
      iSeinLeft = !iSeinLeft;
      iSeinRight = 0;
    } else if (sw->list[SW_K_SEIN_RIGHT].state) {
      // right sein
      tickSein = osKernelGetTickCount();
      iSeinLeft = 0;
      iSeinRight = !iSeinRight;
    } else {
      iSeinLeft = 0;
      iSeinRight = iSeinLeft;
    }
  }

  // set message
  CB.tx.data.u8[0] = sw->list[SW_K_ABS].state;
  CB.tx.data.u8[0] |= _L(db->hmi1.status.mirroring, 1);
  CB.tx.data.u8[0] |= _L(sw->list[SW_K_LAMP].state, 2);
  // FIXME: handle me with real data
  CB.tx.data.u8[0] |= _L(1, 3);
  CB.tx.data.u8[0] |= _L(iStatus.temperature, 4);
  CB.tx.data.u8[0] |= _L(iStatus.finger, 5);
  CB.tx.data.u8[0] |= _L(iStatus.keyless, 6);
  CB.tx.data.u8[0] |= _L(db->hmi1.status.daylight, 7);

  // sein value
  CB.tx.data.u8[1] = iSeinLeft;
  CB.tx.data.u8[1] |= _L(iSeinRight, 1);
  CB.tx.data.u8[1] |= _L(!db->vcu.knob, 2);
  CB.tx.data.u8[2] = db->vcu.signal_percent;
  CB.tx.data.u8[3] = db->bms.soc;

  // odometer
  CB.tx.data.u32[1] = db->vcu.odometer_mps / 1000;

  // set default header
  CANBUS_Header(&(CB.tx.header), CAND_VCU_SWITCH, 8);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_VCU_RTC(timestamp_t *timestamp) {
  // set message
  CB.tx.data.u8[0] = timestamp->time.Seconds;
  CB.tx.data.u8[1] = timestamp->time.Minutes;
  CB.tx.data.u8[2] = timestamp->time.Hours;
  CB.tx.data.u8[3] = timestamp->date.Date;
  CB.tx.data.u8[4] = timestamp->date.Month;
  CB.tx.data.u8[5] = timestamp->date.Year;
  CB.tx.data.u8[6] = timestamp->date.WeekDay;

  // set default header
  CANBUS_Header(&(CB.tx.header), CAND_VCU_RTC, 7);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_VCU_SelectSet(sw_runner_t *runner) {
  static TickType_t tick, tickPeriod;
  static uint8_t iHide = 0;
  static int8_t iName = -1, iValue = -1;

  // Mode Show/Hide Manipulator
  if (runner->listening) {
    // if mode same
    if (iName != runner->mode.val) {
      iName = runner->mode.val;
      // reset period tick
      tickPeriod = osKernelGetTickCount();
    } else if (iValue != runner->mode.sub.val[runner->mode.val]) {
      iValue = runner->mode.sub.val[runner->mode.val];
      // reset period tick
      tickPeriod = osKernelGetTickCount();
    }

    if ((osKernelGetTickCount() - tickPeriod) >= pdMS_TO_TICKS(5000) ||
        (runner->mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R)) {
      // stop listening
      runner->listening = 0;
      iHide = 0;
      iName = -1;
      iValue = -1;
    } else {
      // blink
      if ((osKernelGetTickCount() - tick) >= pdMS_TO_TICKS(250)) {
        tick = osKernelGetTickCount();
        iHide = !iHide;
      }
    }
  } else {
    iHide = 0;
  }

  // set message
  CB.tx.data.u8[0] = runner->mode.sub.val[SW_M_DRIVE];
  CB.tx.data.u8[0] |= _L(runner->mode.sub.val[SW_M_TRIP], 2);
  CB.tx.data.u8[0] |= _L(runner->mode.sub.val[SW_M_REPORT], 3);
  CB.tx.data.u8[0] |= _L(runner->mode.val, 4);

  // Send Show/Hide flag
  CB.tx.data.u8[0] |= _L(iHide, 6);

  CB.tx.data.u8[1] = runner->mode.sub.report[SW_M_REPORT_RANGE];
  CB.tx.data.u8[2] = runner->mode.sub.report[SW_M_REPORT_AVERAGE];

  // set default header
  CANBUS_Header(&(CB.tx.header), CAND_VCU_SELECT_SET, 3);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_VCU_TripMode(uint32_t *trip) {
  // set message
  CB.tx.data.u32[0] = trip[SW_M_TRIP_A];
  CB.tx.data.u32[1] = trip[SW_M_TRIP_B];

  // set default header
  CANBUS_Header(&(CB.tx.header), CAND_VCU_TRIP_MODE, 8);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CANT_BMS_Setting(uint8_t start, BMS_STATE state) {
  // set message
  CB.tx.data.u8[0] = start;
  CB.tx.data.u8[0] |= _L(state, 1);

  // set default header
  CANBUS_Header(&(CB.tx.header), CAND_BMS_SETTING, 1);
  // send message
  return CANBUS_Write(&(CB.tx));
}

/* ------------------------------------ READER ------------------------------------- */
void CANR_BMS_Param1(db_t *db) {
  uint8_t index = DB_BMS_GetIndex(CB.rx.header.ExtId & BMS_ID_MASK);

  // read the content
  db->bms.pack[index].voltage = CB.rx.data.u16[0] * 0.01;
  db->bms.pack[index].current = (CB.rx.data.u16[1] * 0.01) - 50;
  db->bms.pack[index].soc = CB.rx.data.u16[2];
  db->bms.pack[index].temperature = (CB.rx.data.u16[3] * 0.1) - 40;

  // read the id
  db->bms.pack[index].id = CB.rx.header.ExtId & BMS_ID_MASK;
  db->bms.pack[index].started = 1;
  db->bms.pack[index].tick = osKernelGetTickCount();
}

void CANR_BMS_Param2(db_t *db) {
  uint8_t index = DB_BMS_GetIndex(CB.rx.header.ExtId & BMS_ID_MASK);

  // save flag
  db->bms.pack[index].flag = CB.rx.data.u16[3];

  // save state
  db->bms.pack[index].state = _L(_R1(CB.rx.data.u8[7], 4), 1) | _R1(CB.rx.data.u8[7], 5);
}

void CANR_MCU_Dummy(db_t *db) {
  //  FIXME: handle with real data, disable from GPS_CalculateOdometer()
  uint32_t DB_MCU_RPM;

  // read message
  DB_MCU_RPM = CB.rx.data.s64;
  // convert RPM to Speed
  db->vcu.speed = DB_MCU_RPM * MCU_SPEED_MAX / MCU_RPM_MAX;
  // convert Speed to Volume
  db->vcu.volume = db->vcu.speed * 100 / MCU_SPEED_MAX;
}

void CANR_HMI2(db_t *db) {
  // read message
  db->hmi1.status.mirroring = _R1(CB.rx.data.u8[0], 0);

  // save state
  db->hmi2.started = 1;
  db->hmi2.tick = osKernelGetTickCount();
}

void CANR_HMI1_LEFT(db_t *db) {
  // save state
  db->hmi1.device[HMI1_DEV_LEFT].started = 1;
  db->hmi1.device[HMI1_DEV_LEFT].tick = osKernelGetTickCount();
}

void CANR_HMI1_RIGHT(db_t *db) {
  // save state
  db->hmi1.device[HMI1_DEV_RIGHT].started = 1;
  db->hmi1.device[HMI1_DEV_RIGHT].tick = osKernelGetTickCount();
}

