/*
 * _aes.c
 *
 *  Created on: May 12, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_aes.h"

/* Public variable -----------------------------------------------------------*/
__ALIGN_BEGIN uint32_t AesKey[4] __ALIGN_END;

/* Private variable ----------------------------------------------------------*/
static aes_t aes;

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation -------------------------------------------*/
uint8_t AES_Init(CRYP_HandleTypeDef *hcryp, osMutexId_t mutex) {
	uint8_t ok = 0;

  aes.h.cryp = hcryp;
  aes.h.mutex = mutex;

  do {
  	ok = AES_ChangeKey(NULL);
    _DelayMS(100);
  } while (!ok);

  return ok;
}

uint8_t AES_ChangeKey(uint32_t *key) {
  CRYP_ConfigTypeDef config;
  HAL_StatusTypeDef ret;

  lock();
  ret = HAL_CRYP_GetConfig(aes.h.cryp, &config);
  if (ret == HAL_OK) {
    config.pKey = (key == NULL) ? AesKey : key;
    HAL_CRYP_SetConfig(aes.h.cryp, &config);
  }
  unlock();

  return (ret == HAL_OK);
}

uint8_t AES_Encrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz) {
  uint8_t ret;

  lock();
  ret = (HAL_CRYP_Encrypt(aes.h.cryp, (uint32_t*) pSrc, Sz, (uint32_t*) pDst, 1000) == HAL_OK);
  unlock();

  return ret;
}

uint8_t AES_Decrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz) {
  uint8_t ret;

  lock();
  ret = (HAL_CRYP_Decrypt(aes.h.cryp, (uint32_t*) pSrc, Sz, (uint32_t*) pDst, 1000) == HAL_OK);
  unlock();

  return ret;
}
/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  osMutexAcquire(aes.h.mutex, osWaitForever);
}

static void unlock(void) {
  osMutexRelease(aes.h.mutex);
}
