/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_database.h"
#include "_reporter.h"

/* Public variables -----------------------------------------------------------*/
db_t DB;

/* Public functions implementation --------------------------------------------*/
void DB_Init(void) {
  // reset VCU data
  DB.vcu.independent = 1;
  DB.vcu.interval = RPT_INTERVAL_SIMPLE;
  DB.vcu.volume = 0;
  DB.vcu.bat_voltage = 0;
  DB.vcu.signal_percent = 0;
  DB.vcu.speed = 0;
  DB.vcu.odometer = 0;
  DB.vcu.events = 0;
  DB.vcu.seq_id.report = 0;
  DB.vcu.seq_id.response = 0;

  // reset HMI1 data
  DB.hmi1.status.mirroring = 0;
  DB.hmi1.status.warning = 0;
  DB.hmi1.status.temperature = 0;
  DB.hmi1.status.finger = 0;
  DB.hmi1.status.keyless = 0;
  DB.hmi1.status.daylight = 0;

  // reset HMI2 data
  DB.hmi2.shutdown = 0;

  // reset BMS data
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    DB_BMS_ResetIndex(i);
  }
}

void DB_BMS_CheckIndex(void) {
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    if ((osKernelGetTickCount() - DB.bms.pack[i].tick) > pdMS_TO_TICKS(500)) {
      DB_BMS_ResetIndex(i);
    }
  }
}

uint8_t DB_BMS_GetIndex(uint32_t id) {
  uint8_t i;

  // find index (if already exist)
  for (i = 0; i < BMS_COUNT; i++) {
    if (DB.bms.pack[i].id == id) {
      return i;
    }
  }

  // finx index (if not exist)
  for (i = 0; i < BMS_COUNT; i++) {
    if (DB.bms.pack[i].id == BMS_NULL_INDEX) {
      return i;
    }
  }

  // force replace first index (if already full)
  return 0;
}

uint8_t DB_BMS_CheckRun(uint8_t state) {
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    if (DB.bms.pack[i].started != state) {
      return 0;
    }
  }
  return 1;
}

uint8_t DB_BMS_CheckState(BMS_STATE state) {
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    if (DB.bms.pack[i].state != state) {
      return 0;
    }
  }
  return 1;
}

void DB_BMS_MergeFlags(void) {
  uint16_t flags = 0;

  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    flags |= DB.bms.pack[i].flag;
  }
  // apply to events
  DB_BMS_Events(flags);
}

void DB_BMS_ResetIndex(uint8_t i) {
  DB.bms.pack[i].id = BMS_NULL_INDEX;
  DB.bms.pack[i].voltage = 0;
  DB.bms.pack[i].current = 0;
  DB.bms.pack[i].soc = 0;
  DB.bms.pack[i].temperature = 0;
  DB.bms.pack[i].state = BMS_STATE_IDLE;
  DB.bms.pack[i].started = 0;
  DB.bms.pack[i].flag = 0;
  DB.bms.pack[i].tick = 0;
}

void DB_VCU_CheckIndependent(void) {
  DB.vcu.independent = !HAL_GPIO_ReadPin(EXT_BMS_IRQ_GPIO_Port, EXT_BMS_IRQ_Pin);
  DB.vcu.interval = DB.vcu.independent ? RPT_INTERVAL_INDEPENDENT : RPT_INTERVAL_SIMPLE;
  DB_SetEvent(EV_VCU_INDEPENDENT, DB.vcu.independent);
}

void DB_SetEvent(uint64_t event_id, uint8_t value) {
  if (value & 1) {
    BV(DB.vcu.events, _BitPosition(event_id));
  } else {
    BC(DB.vcu.events, _BitPosition(event_id));
  }
}

void DB_BMS_Events(uint16_t flag) {
  DB_SetEvent(EV_BMS_SHORT_CIRCUIT, _R1(flag, 0));
  DB_SetEvent(EV_BMS_DISCHARGE_OVER_CURRENT, _R1(flag, 1));
  DB_SetEvent(EV_BMS_CHARGE_OVER_CURRENT, _R1(flag, 2));
  DB_SetEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE, _R1(flag, 3));
  DB_SetEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE, _R1(flag, 4));
  DB_SetEvent(EV_BMS_CHARGE_OVER_TEMPERATURE, _R1(flag, 5));
  DB_SetEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE, _R1(flag, 6));
  DB_SetEvent(EV_BMS_UNBALANCE, _R1(flag, 7));
  DB_SetEvent(EV_BMS_UNDER_VOLTAGE, _R1(flag, 8));
  DB_SetEvent(EV_BMS_OVER_VOLTAGE, _R1(flag, 9));
  DB_SetEvent(EV_BMS_OVER_DISCHARGE_CAPACITY, _R1(flag, 10));
  DB_SetEvent(EV_BMS_SYSTEM_FAILURE, _R1(flag, 11));
}

//void DB_SetEvents(uint64_t value) {
//  DB.vcu.events = value;
//}
//
//uint8_t DB_ReadEvent(uint64_t event_id) {
//  return _R8((DB.vcu.events & event_id), _BitPosition(event_id));
//}

