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
  if ((osKernelSysTick() - tick) >= pdMS_TO_TICKS(500)) {
    // finger
    if (db->hmi1.status.finger) {
      tick = osKernelSysTick();
      iStatus.finger = !iStatus.finger;
    }
    // keyless
    if (db->hmi1.status.keyless) {
      tick = osKernelSysTick();
      iStatus.keyless = !iStatus.keyless;
    }
    // temperature
    if (db->hmi1.status.temperature) {
      tick = osKernelSysTick();
      iStatus.temperature = !iStatus.temperature;
    }
  }

  // sein manipulator
  if ((osKernelSysTick() - tickSein) >= pdMS_TO_TICKS(500)) {
    if (sw->list[SW_K_SEIN_LEFT].state && sw->list[SW_K_SEIN_RIGHT].state) {
      // hazard
      tickSein = osKernelSysTick();
      iSeinLeft = !iSeinLeft;
      iSeinRight = iSeinLeft;
    } else if (sw->list[SW_K_SEIN_LEFT].state) {
      // left sein
      tickSein = osKernelSysTick();
      iSeinLeft = !iSeinLeft;
      iSeinRight = 0;
    } else if (sw->list[SW_K_SEIN_RIGHT].state) {
      // right sein
      tickSein = osKernelSysTick();
      iSeinLeft = 0;
      iSeinRight = !iSeinRight;
    } else {
      iSeinLeft = 0;
      iSeinRight = iSeinLeft;
    }
  }

  // set message
  CB.tx.data.u8[0] = sw->list[SW_K_ABS].state;
  // FIXME: handle me with real data
  CB.tx.data.u8[0] |= _L(db->hmi1.status.mirroring, 1);
  CB.tx.data.u8[0] |= _L(sw->list[SW_K_LAMP].state, 2);
  CB.tx.data.u8[0] |= _L(1, 3);
  CB.tx.data.u8[0] |= _L(iStatus.temperature, 4);
  CB.tx.data.u8[0] |= _L(iStatus.finger, 5);
  CB.tx.data.u8[0] |= _L(iStatus.keyless, 6);
  CB.tx.data.u8[0] |= _L(db->hmi1.status.daylight, 7);

  // sein value
  CB.tx.data.u8[1] = iSeinLeft;
  CB.tx.data.u8[1] |= _L(iSeinRight, 1);
  CB.tx.data.u8[1] |= _L(db->hmi2.shutdown, 2);
  CB.tx.data.u8[2] = db->vcu.signal_percent;

  // odometer
  CB.tx.data.u32[1] = db->vcu.odometer;

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
      tickPeriod = osKernelSysTick();
    } else if (iValue != runner->mode.sub.val[runner->mode.val]) {
      iValue = runner->mode.sub.val[runner->mode.val];
      // reset period tick
      tickPeriod = osKernelSysTick();
    }

    if ((osKernelSysTick() - tickPeriod) >= pdMS_TO_TICKS(5000) ||
        (runner->mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R)) {
      // stop listening
      runner->listening = 0;
      iHide = 0;
      iName = -1;
      iValue = -1;
    } else {
      // blink
      if ((osKernelSysTick() - tick) >= pdMS_TO_TICKS(250)) {
        tick = osKernelSysTick();
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

uint8_t CANT_BMS_StateSetting(uint8_t start, BMS_STATE state) {
  // set message
  CB.tx.data.u8[0] = start;
  CB.tx.data.u8[0] |= _L(state, 1);

  // set default header
  CANBUS_Header(&(CB.tx.header), CAND_BMS_STATE_SETTING, 1);
  // send message
  return CANBUS_Write(&(CB.tx));
}

/* ------------------------------------ READER ------------------------------------- */
void CANR_BMS_Param1(db_t *db) {
  db->bms.pack[0].voltage = CB.rx.data.u16[0] * 0.001;
  db->bms.pack[0].current = (CB.rx.data.u16[1] * 0.01) + 50;
  db->bms.pack[0].soc = CB.rx.data.u16[2];
  db->bms.pack[0].temperature = CB.rx.data.u16[3] * 0.1;
  // read the id
  db->bms.pack[0].id = CB.rx.header.ExtId & 0xFFFFF;
}

void CANR_BMS_Param2(db_t *db) {
  RPT_SetEvent(RPT_BMS_SHORT_CIRCUIT, _R1(CB.rx.data.u8[6], 0));
  RPT_SetEvent(RPT_BMS_DISCHARGE_OVER_CURRENT, _R1(CB.rx.data.u8[6], 1));
  RPT_SetEvent(RPT_BMS_CHARGE_OVER_CURRENT, _R1(CB.rx.data.u8[6], 2));
  RPT_SetEvent(RPT_BMS_DISCHARGE_OVER_TEMPERATURE, _R1(CB.rx.data.u8[6], 3));
  RPT_SetEvent(RPT_BMS_DISCHARGE_UNDER_TEMPERATURE, _R1(CB.rx.data.u8[6], 4));
  RPT_SetEvent(RPT_BMS_CHARGE_OVER_TEMPERATURE, _R1(CB.rx.data.u8[6], 5));
  RPT_SetEvent(RPT_BMS_CHARGE_UNDER_TEMPERATURE, _R1(CB.rx.data.u8[6], 6));
  RPT_SetEvent(RPT_BMS_UNBALANCE, _R1(CB.rx.data.u8[6], 7));
  RPT_SetEvent(RPT_BMS_UNDER_VOLTAGE, _R1(CB.rx.data.u8[7], 0));
  RPT_SetEvent(RPT_BMS_OVER_VOLTAGE, _R1(CB.rx.data.u8[7], 1));
  RPT_SetEvent(RPT_BMS_OVER_DISCHARGE_CAPACITY, _R1(CB.rx.data.u8[7], 2));
  RPT_SetEvent(RPT_BMS_SYSTEM_FAILURE, _R1(CB.rx.data.u8[7], 3));

  // read state
  db->bms.pack[0].state.charge = _R1(CB.rx.data.u8[7], 4);
  db->bms.pack[0].state.discharge = _R1(CB.rx.data.u8[7], 5);
  db->bms.pack[0].state.idle = _R1(CB.rx.data.u8[7], 6);

  // save state
  db->bms.state = _L(db->bms.pack[0].state.charge, 1) | db->bms.pack[0].state.discharge;
  db->bms.start = 1;
}

void CANR_MCU_Dummy(db_t *db) {
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
}

