/*
 * _aes.c
 *
 *  Created on: May 12, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_aes.h"

#include "Libs/_eeprom.h"
#include "aes.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t AesMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static __ALIGN_BEGIN aes_key_t AES_KEY __ALIGN_END;
static CRYP_HandleTypeDef *pcryp = &hcryp;

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t AES_Init(void) {
  uint8_t ok;

  AES_EE_Key(NULL);

  do {
    ok = AES_ChangeKey(NULL);
    if (!ok) _DelayMS(100);
  } while (!ok);

  return ok;
}

uint8_t AES_ChangeKey(uint32_t *src) {
  CRYP_ConfigTypeDef config;
  uint8_t ok;

  lock();
  ok = HAL_CRYP_GetConfig(pcryp, &config) == HAL_OK;
  if (ok) {
    config.pKey = (src == NULL) ? AES_KEY : src;
    ok = HAL_CRYP_SetConfig(pcryp, &config) == HAL_OK;
  }
  unlock();

  return ok;
}

uint8_t AES_Encrypt(uint8_t *dst, const uint8_t *src, uint16_t Sz) {
  uint8_t ok;

  lock();
  ok = (HAL_CRYP_Encrypt(pcryp, (uint32_t *)src, Sz, (uint32_t *)dst, 1000) ==
        HAL_OK);
  unlock();

  return ok;
}

uint8_t AES_Decrypt(uint8_t *dst, const uint8_t *src, uint16_t Sz) {
  uint8_t ok;

  lock();
  ok = (HAL_CRYP_Decrypt(pcryp, (uint32_t *)src, Sz, (uint32_t *)dst, 1000) ==
        HAL_OK);
  unlock();

  return ok;
}

uint8_t AES_EE_Key(const aes_key_t src) {
  void *dst = AES_KEY;

  return EE_Cmd(VA_AES_KEY, src, dst);
}

uint32_t AES_IO_QuarterKey(void) { return AES_KEY[0]; }
/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(AesMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(AesMutexHandle);
#endif
}
