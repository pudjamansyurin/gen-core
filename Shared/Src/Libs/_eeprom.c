/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_eeprom.h"

#include "Drivers/_ee24xx.h"
#include "i2c.h"

#if (APP)
#include "Drivers/_aes.h"
#include "Libs/_hbar.h"
#else
#include "App/_fota.h"
#endif
#include "Drivers/_sim_con.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t EepromRecMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static I2C_HandleTypeDef* pi2c = &hi2c2;

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t EE_Init(void) {
  uint8_t ok;

  lock();
  printf("EEPROM:Init\n");
  EEPROM24XX_SetDevice(pi2c, EE_ADDR);
  ok = EEPROM24XX_IsConnected(100);

  if (ok) {
#if (APP)
    AES_KeyStore(NULL);
    HBAR_LoadStore();
#else
    if (IEEP_VALUE == IEEP_RESET) {
      if (SIMCon_SetDefaultStore()) {
        uint32_t ieep = IEEP_SET;
        IAP_SetBootMeta(IEEP_OFFSET, &ieep);
      }
    }
#endif
    SIMCon_LoadStore();
    IAP_VersionStore(NULL);
    IAP_TypeStore(NULL);
  }
  unlock();

  if (!ok) printf("EEPROM:Error\n");
  return ok;
}

uint8_t EE_Cmd(EE_VA va, void* src, void* dst, uint16_t size) {
  uint16_t addr = EE_WORD(va);
  uint8_t ok;

  lock();
  if (src == NULL) {
    ok = EEPROM24XX_Load(addr, dst, size);
  } else {
    memcpy(dst, src, size);
    ok = EEPROM24XX_Reset(addr);
    ok = EEPROM24XX_Save(addr, src, size);
  }
  unlock();

  return ok;
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(EepromRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(EepromRecMutexHandle);
#endif
}
