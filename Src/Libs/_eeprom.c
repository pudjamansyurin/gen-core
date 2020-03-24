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
  EEPROM24XX_SetDevice(EEPROM24_MAIN);
  if (EEPROM24XX_IsConnected()) {
    LOG_StrLn("MAIN EEPROM is used.");
    return 1;
  } else {
    EEPROM24XX_SetDevice(EEPROM24_BACKUP);
    if (EEPROM24XX_IsConnected()) {
      LOG_StrLn("BACKUP EEPROM is used.");
      return 1;
    }
  }
  LOG_StrLn("All EEPROM is failed.");
  return 0;
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
