/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_database.h"

/* Public variables -----------------------------------------------------------*/
db_t DB = {
    .vcu = {
        .independent = 1,
        .interval = RPT_INTERVAL_SIMPLE,
        .volume = 0,
        .bat_voltage = 0,
        .signal_percent = 0,
        .speed = 0,
        .odometer = 0,
        .rtc = {
            .calibration = { 0 },
            .timestamp = {
                .time = { 0 },
                .date = { 0 }
            },
        },
    },
    .hmi1 = {
        .status = {
            .mirroring = 0,
            .warning = 1,
            .temperature = 1,
            .finger = 1,
            .keyless = 1,
            .daylight = 1
        }
    },
    .hmi2 = {
        .shutdown = 0
    },
    .bms = { 0 }
};

/* Public functions implementation --------------------------------------------*/
void DB_Init(void) {
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    DB_BMS_ResetIndex(i);
  }
}

void DB_BMS_CheckIndex(void) {
  for (uint8_t i = 0; i < BMS_COUNT; i++) {
    if ((osKernelSysTick() - DB.bms.pack[i].tick) > pdMS_TO_TICKS(1000)) {
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

  DB.bms.flags = flags;
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

