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

/* Public variables
 * --------------------------------------------*/
__ALIGN_BEGIN uint32_t AES_KEY[4] __ALIGN_END;

/* Private variables
 * --------------------------------------------*/
static CRYP_HandleTypeDef *pcryp = &hcryp;

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t AES_Init(void) {
  uint8_t ok;

  do {
    ok = AES_ChangeKey(NULL);
    if (!ok) _DelayMS(100);
  } while (!ok);

  return ok;
}

uint8_t AES_ChangeKey(uint32_t *key) {
  CRYP_ConfigTypeDef config;
  uint8_t ok;

  lock();
  ok = HAL_CRYP_GetConfig(pcryp, &config) == HAL_OK;
  if (ok) {
    config.pKey = (key == NULL) ? AES_KEY : key;
    HAL_CRYP_SetConfig(pcryp, &config);
  }
  unlock();

  return ok;
}

uint8_t AES_Encrypt(uint8_t *dst, uint8_t *src, uint16_t Sz) {
  uint8_t ok;

  lock();
  ok = (HAL_CRYP_Encrypt(pcryp, (uint32_t *)src, Sz, (uint32_t *)dst, 1000) ==
        HAL_OK);
  unlock();

  return ok;
}

uint8_t AES_Decrypt(uint8_t *dst, uint8_t *src, uint16_t Sz) {
  uint8_t ok;

  lock();
  ok = (HAL_CRYP_Decrypt(pcryp, (uint32_t *)src, Sz, (uint32_t *)dst, 1000) ==
        HAL_OK);
  unlock();

  return ok;
}

uint8_t AES_KeyStore(uint32_t src[4]) {
  void *dst = AES_KEY;

  return EE_Cmd(VA_AES_KEY, src, dst);
}

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
