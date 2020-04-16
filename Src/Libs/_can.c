/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_can.h"

/* External variables ----------------------------------------------------------*/
extern canbus_t CB;

/* Public functions implementation --------------------------------------------*/
uint8_t CAN_VCU_Switch(db_t *db) {
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
    if (db->vcu.sw.list[SW_K_SEIN_LEFT].state && db->vcu.sw.list[SW_K_SEIN_RIGHT].state) {
      // hazard
      tickSein = osKernelSysTick();
      iSeinLeft = !iSeinLeft;
      iSeinRight = iSeinLeft;
    } else if (db->vcu.sw.list[SW_K_SEIN_LEFT].state) {
      // left sein
      tickSein = osKernelSysTick();
      iSeinLeft = !iSeinLeft;
      iSeinRight = 0;
    } else if (db->vcu.sw.list[SW_K_SEIN_RIGHT].state) {
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
  CB.tx.data[0] = db->vcu.sw.list[SW_K_ABS].state;
  // FIXME: handle me with real data
  CB.tx.data[0] |= BSL(db->hmi1.status.mirroring, 1);
  CB.tx.data[0] |= BSL(db->vcu.sw.list[SW_K_LAMP].state, 2);
  CB.tx.data[0] |= BSL(1, 3);
  CB.tx.data[0] |= BSL(iStatus.temperature, 4);
  CB.tx.data[0] |= BSL(iStatus.finger, 5);
  CB.tx.data[0] |= BSL(iStatus.keyless, 6);
  CB.tx.data[0] |= BSL(db->hmi1.status.daylight, 7);

  // sein value
  CB.tx.data[1] = iSeinLeft;
  CB.tx.data[1] |= BSL(iSeinRight, 1);

  // HMI-2 Shutdown Request
  CB.tx.data[1] |= BSL(db->hmi2.shutdown, 2);

  // signal strength
  CB.tx.data[2] = db->vcu.signal;

  // odometer
  CB.tx.data[4] = BSR(db->vcu.odometer, 0);
  CB.tx.data[5] = BSR(db->vcu.odometer, 8);
  CB.tx.data[6] = BSR(db->vcu.odometer, 16);
  CB.tx.data[7] = BSR(db->vcu.odometer, 24);

  // set default header
  CANBUS_Header(&(CB.tx.header), CAN_ADDR_VCU_SWITCH, 8);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CAN_VCU_RTC(timestamp_t *timestamp) {
  // set message
  CB.tx.data[0] = timestamp->time.Seconds;
  CB.tx.data[1] = timestamp->time.Minutes;
  CB.tx.data[2] = timestamp->time.Hours;
  CB.tx.data[3] = timestamp->date.Date;
  CB.tx.data[4] = timestamp->date.Month;
  CB.tx.data[5] = timestamp->date.Year;
  CB.tx.data[6] = timestamp->date.WeekDay;

  // set default header
  CANBUS_Header(&(CB.tx.header), CAN_ADDR_VCU_RTC, 7);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CAN_VCU_Select_Set(sw_runner_t *runner) {
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
  CB.tx.data[0] = runner->mode.sub.val[SW_M_DRIVE];
  CB.tx.data[0] |= BSL(runner->mode.sub.val[SW_M_TRIP], 2);
  CB.tx.data[0] |= BSL(runner->mode.sub.val[SW_M_REPORT], 3);
  CB.tx.data[0] |= BSL(runner->mode.val, 4);

  // Send Show/Hide flag
  CB.tx.data[0] |= BSL(iHide, 6);

  CB.tx.data[1] = runner->mode.sub.report[SW_M_REPORT_RANGE];
  CB.tx.data[2] = runner->mode.sub.report[SW_M_REPORT_AVERAGE];

  // set default header
  CANBUS_Header(&(CB.tx.header), CAN_ADDR_VCU_SELECT_SET, 3);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CAN_VCU_Trip_Mode(uint32_t *trip) {
  // set message
  CB.tx.data[0] = BSR(trip[SW_M_TRIP_A], 0);
  CB.tx.data[1] = BSR(trip[SW_M_TRIP_A], 8);
  CB.tx.data[2] = BSR(trip[SW_M_TRIP_A], 16);
  CB.tx.data[3] = BSR(trip[SW_M_TRIP_A], 24);
  CB.tx.data[4] = BSR(trip[SW_M_TRIP_B], 0);
  CB.tx.data[5] = BSR(trip[SW_M_TRIP_B], 8);
  CB.tx.data[6] = BSR(trip[SW_M_TRIP_B], 16);
  CB.tx.data[7] = BSR(trip[SW_M_TRIP_B], 24);

  // set default header
  CANBUS_Header(&(CB.tx.header), CAN_ADDR_VCU_TRIP_MODE, 8);
  // send message
  return CANBUS_Write(&(CB.tx));
}

/* ------------------------------------ READER ------------------------------------- */
void CAN_MCU_Dummy_Read(db_t *db) {
  uint32_t DB_MCU_RPM;

  // read message
  DB_MCU_RPM = CB.rx.data[0] | BSL(CB.rx.data[1], 8) | BSL(CB.rx.data[2], 16) | BSL(CB.rx.data[3], 24);

  // convert RPM to Speed
  db->vcu.speed = DB_MCU_RPM * MCU_SPEED_MAX / MCU_RPM_MAX;

  // convert Speed to Volume
  db->vcu.volume = db->vcu.speed * 100 / MCU_SPEED_MAX;
}

void CAN_HMI2_Read(db_t *db) {
  // read message
  db->hmi1.status.mirroring = BBR(CB.rx.data[0], 0);
}

