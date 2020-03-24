/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_eeprom.h"

/* Private functions prototype ------------------------------------------------*/
static uint8_t EE_32(uint16_t vaddr, EEPROM_COMMAND cmd, uint32_t *value);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(void) {
  uint8_t retry = 5, ret = 0;

  // check main eeprom
  EEPROM24XX_SetDevice(EEPROM24_MAIN);
  do {
    if (EEPROM24XX_IsConnected()) {
      LOG_StrLn("MAIN EEPROM is used.");
      ret = 1;
      break;
    }
    osDelay(50);
  } while (retry--);
  // check backup eeprom
  retry = 5;
  EEPROM24XX_SetDevice(EEPROM24_BACKUP);
  do {
    if (EEPROM24XX_IsConnected()) {
      LOG_StrLn("MAIN EEPROM is used.");
      ret = 1;
      break;
    }
    osDelay(50);
  } while (retry--);
  // all failed
  if (!ret) {
    LOG_StrLn("All EEPROM are failed.");
  }

  return ret;
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t *value) {
  return EE_32(VADDR_ODOMETER, cmd, value);
}

uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t *value) {
  return EE_32(VADDR_UNITID, cmd, value);
}

/* Private functions implementation --------------------------------------------*/
static uint8_t EE_32(uint16_t vaddr, EEPROM_COMMAND cmd, uint32_t *value) {
  if (cmd == EE_CMD_W) {
    return EEPROM24XX_Save(vaddr, value, sizeof(uint32_t));
  }
  return EEPROM24XX_Load(vaddr, value, sizeof(uint32_t));
}
