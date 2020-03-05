/*
 * _canbus.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#include "_can.h"
#include "_rtc.h"

extern const TickType_t tick5000ms,
    tick500ms,
    tick250ms;
extern db_t DB;
extern canbus_t CB;

// ==================================== VCU =========================================
#if (CAN_NODE & CAN_NODE_VCU)
uint8_t CAN_VCU_Switch(void) {
  static TickType_t tick, tickSein;
  static uint8_t iSein_Left = 0, iSein_Right = 0;
  static status_t iDB_hmi1_Status;

  // indicator manipulator
  if ((osKernelSysTick() - tick) >= tick500ms) {
    // finger
    if (DB.hmi1.status.finger) {
      tick = osKernelSysTick();
      iDB_hmi1_Status.finger = !iDB_hmi1_Status.finger;
    }
    // keyless
    if (DB.hmi1.status.keyless) {
      tick = osKernelSysTick();
      iDB_hmi1_Status.keyless = !iDB_hmi1_Status.keyless;
    }
    // temperature
    if (DB.hmi1.status.temperature) {
      tick = osKernelSysTick();
      iDB_hmi1_Status.temperature = !iDB_hmi1_Status.temperature;
    }
  }

  // sein manipulator
  if ((osKernelSysTick() - tickSein) >= tick500ms) {
    if (DB.vcu.sw.list[SW_K_SEIN_LEFT].state && DB.vcu.sw.list[SW_K_SEIN_RIGHT].state) {
      // hazard
      tickSein = osKernelSysTick();
      iSein_Left = !iSein_Left;
      iSein_Right = iSein_Left;
    } else if (DB.vcu.sw.list[SW_K_SEIN_LEFT].state) {
      // left sein
      tickSein = osKernelSysTick();
      iSein_Left = !iSein_Left;
      iSein_Right = 0;
    } else if (DB.vcu.sw.list[SW_K_SEIN_RIGHT].state) {
      // right sein
      tickSein = osKernelSysTick();
      iSein_Left = 0;
      iSein_Right = !iSein_Right;
    } else {
      iSein_Left = 0;
      iSein_Right = iSein_Left;
    }
  }

  // set message
  CB.tx.data[0] = DB.vcu.sw.list[SW_K_ABS].state;
  CB.tx.data[0] |= DB.vcu.sw.list[SW_K_MIRRORING].state << 1;
  CB.tx.data[0] |= 1 << 2;
  CB.tx.data[0] |= 1 << 3;
  CB.tx.data[0] |= iDB_hmi1_Status.temperature << 4;
  CB.tx.data[0] |= iDB_hmi1_Status.finger << 5;
  CB.tx.data[0] |= iDB_hmi1_Status.keyless << 6;
  // check daylight (for auto brightness of HMI)
  CB.tx.data[0] |= RTC_Daylight() << 7;

  // sein value
  CB.tx.data[1] = iSein_Left << 0;
  CB.tx.data[1] |= iSein_Right << 1;

  // HMI-2 Shutdown Request
  CB.tx.data[1] |= DB.hmi2.shutdown << 2;

  // signal strength
  CB.tx.data[2] = DB.vcu.signal;

  // odometer
  CB.tx.data[4] = (DB.vcu.odometer >> 0) & 0xFF;
  CB.tx.data[5] = (DB.vcu.odometer >> 8) & 0xFF;
  CB.tx.data[6] = (DB.vcu.odometer >> 16) & 0xFF;
  CB.tx.data[7] = (DB.vcu.odometer >> 24) & 0xFF;

  // dummy algorithm
  DB.vcu.odometer = (DB.vcu.odometer >= VCU_ODOMETER_MAX ? 0 : (DB.vcu.odometer + 1));

  // set default header
  CANBUS_Set_Tx_Header(&(CB.tx.header), CAN_ADDR_VCU_SWITCH, 8);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CAN_VCU_RTC(void) {
  // set message
  RTC_Read_RAW(&DB.vcu.timestamp);
  CB.tx.data[0] = DB.vcu.timestamp.time.Seconds;
  CB.tx.data[1] = DB.vcu.timestamp.time.Minutes;
  CB.tx.data[2] = DB.vcu.timestamp.time.Hours;
  CB.tx.data[3] = DB.vcu.timestamp.date.Date;
  CB.tx.data[4] = DB.vcu.timestamp.date.Month;
  CB.tx.data[5] = DB.vcu.timestamp.date.Year;
  CB.tx.data[6] = DB.vcu.timestamp.date.WeekDay;

  // set default header
  CANBUS_Set_Tx_Header(&(CB.tx.header), CAN_ADDR_VCU_RTC, 7);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CAN_VCU_Select_Set(void) {
  static TickType_t tick, tickPeriod;
  static uint8_t iMode_Hide = 0;
  static int8_t iMode_Name = -1, iMode_Value = -1;

  // Mode Show/Hide Manipulator
  if (DB.vcu.sw.runner.listening) {
    // if mode same
    if (iMode_Name != DB.vcu.sw.runner.mode.val) {
      iMode_Name = DB.vcu.sw.runner.mode.val;
      // reset period tick
      tickPeriod = osKernelSysTick();
    } else if (iMode_Value != DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val]) {
      iMode_Value = DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val];
      // reset period tick
      tickPeriod = osKernelSysTick();
    }

    if ((osKernelSysTick() - tickPeriod) >= tick5000ms ||
        (DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R)) {
      // stop listening
      DB.vcu.sw.runner.listening = 0;
      iMode_Hide = 0;
      iMode_Name = -1;
      iMode_Value = -1;
    } else {
      // blink
      if ((osKernelSysTick() - tick) >= tick250ms) {
        tick = osKernelSysTick();
        iMode_Hide = !iMode_Hide;
      }
    }
  } else {
    iMode_Hide = 0;
  }

  // set message
  CB.tx.data[0] = DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE];
  CB.tx.data[0] |= DB.vcu.sw.runner.mode.sub.val[SW_M_TRIP] << 2;
  CB.tx.data[0] |= DB.vcu.sw.runner.mode.sub.val[SW_M_REPORT] << 3;
  CB.tx.data[0] |= DB.vcu.sw.runner.mode.val << 4;

  // Send Show/Hide flag
  CB.tx.data[0] |= iMode_Hide << 6;

  CB.tx.data[1] = DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_RANGE];
  CB.tx.data[2] = DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE];

  // dummy algorithm
  if (!DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_RANGE]) {
    DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_RANGE] = 255;
  } else {
    DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_RANGE]--;
  }

  if (DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE] >= 255) {
    DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE] = 0;
  } else {
    DB.vcu.sw.runner.mode.sub.report[SW_M_REPORT_AVERAGE]++;
  }

  // set default header
  CANBUS_Set_Tx_Header(&(CB.tx.header), CAN_ADDR_VCU_SELECT_SET, 3);
  // send message
  return CANBUS_Write(&(CB.tx));
}

uint8_t CAN_VCU_Trip_Mode(void) {
  // set message
  CB.tx.data[0] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 0) & 0xFF;
  CB.tx.data[1] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 8) & 0xFF;
  CB.tx.data[2] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 16) & 0xFF;
  CB.tx.data[3] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A] >> 24) & 0xFF;
  CB.tx.data[4] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 0) & 0xFF;
  CB.tx.data[5] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 8) & 0xFF;
  CB.tx.data[6] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 16) & 0xFF;
  CB.tx.data[7] = (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B] >> 24) & 0xFF;

  // dummy algorithm
  if (DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val] == SW_M_TRIP_A) {
    if (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A] >= VCU_ODOMETER_MAX) {
      DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A] = 0;
    } else {
      DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_A]++;
    }
  } else {
    if (DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B] >= VCU_ODOMETER_MAX) {
      DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B] = 0;
    } else {
      DB.vcu.sw.runner.mode.sub.trip[SW_M_TRIP_B]++;
    }
  }
  // set default header
  CANBUS_Set_Tx_Header(&(CB.tx.header), CAN_ADDR_VCU_TRIP_MODE, 8);
  // send message
  return CANBUS_Write(&(CB.tx));
}

/* ------------------------------------ READER ------------------------------------- */
void CAN_MCU_Dummy_Read(void) {
  uint32_t DB_MCU_RPM;

  // read message
  DB_MCU_RPM = (CB.rx.data[3] << 24 | CB.rx.data[2] << 16 | CB.rx.data[1] << 8 | CB.rx.data[0]);
  // convert RPM to Speed
  DB.vcu.speed = DB_MCU_RPM * MCU_SPEED_MAX / MCU_RPM_MAX;
}
#endif

