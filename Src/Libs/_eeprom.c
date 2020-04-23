/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_eeprom.h"
#include "_database.h"
#include "_reporter.h"

/* External variables ---------------------------------------------------------*/
extern db_t DB;
extern report_t REPORT;
extern response_t RESPONSE;

/* Private functions prototype ------------------------------------------------*/
static uint8_t EE_32(uint16_t vaddr, EEPROM_COMMAND cmd, uint32_t *value);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(void) {
  uint8_t retry = 5;

  LOG_StrLn("EEPROM:Init");
  // check main eeprom
  EEPROM24XX_SetDevice(EEPROM24_MAIN);
  do {
    if (EEPROM24XX_IsConnected()) {
      LOG_StrLn("EEPROM:Main");
      return 1;
    }
    osDelay(50);
  } while (retry--);

  // check backup eeprom
  retry = 5;
  EEPROM24XX_SetDevice(EEPROM24_BACKUP);
  do {
    if (EEPROM24XX_IsConnected()) {
      LOG_StrLn("EEPROM:Backup");
      return 1;
    }
    osDelay(50);
  } while (retry--);

  // all failed
  LOG_StrLn("EEPROM:Error");
  return 0;
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value) {
  if (DB.vcu.odometer == value) {
    return 1;
  }
  // only update when value is different
  if (EE_32(VADDR_ODOMETER, cmd, &value)) {
    DB.vcu.odometer = value;

    return 1;
  }
  return 0;
}

uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value) {
  if (DB.vcu.unit_id == value) {
    return 1;
  }
  // only update when value is different
  if (EE_32(VADDR_UNITID, cmd, &value)) {
    DB.vcu.unit_id = value;

    return 1;
  }
  return 0;
}

/* Private functions implementation --------------------------------------------*/
static uint8_t EE_32(uint16_t vaddr, EEPROM_COMMAND cmd, uint32_t *value) {
  if (cmd == EE_CMD_W) {
    return EEPROM24XX_Save(vaddr, value, sizeof(uint32_t));
  }
  return EEPROM24XX_Load(vaddr, value, sizeof(uint32_t));
}
