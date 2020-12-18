/*
 * _aes.c
 *
 *  Created on: May 12, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_aes.h"

/* External variable ---------------------------------------------------------*/
extern osMutexId_t AesMutexHandle;

/* Public variable -----------------------------------------------------------*/
__ALIGN_BEGIN uint32_t AesKey[4] __ALIGN_END;

/* Private variable ----------------------------------------------------------*/
static aes_handler_t hAES;

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation -------------------------------------------*/
uint8_t AES_Init(CRYP_HandleTypeDef *hcryp) {
  HAL_StatusTypeDef ret;
  CRYP_ConfigTypeDef config;

  hAES.hcryp = hcryp;

  do {
    lock();
    ret = HAL_CRYP_GetConfig(hAES.hcryp, &config);
    if (ret == HAL_OK) {
      config.pKey = AesKey;
      HAL_CRYP_SetConfig(hAES.hcryp, &config);
    }
    unlock();
    _DelayMS(100);
  } while (ret != HAL_OK);

  return ret;
}

uint8_t AES_Encrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz) {
  uint8_t ret;

  lock();
  ret = (HAL_CRYP_Encrypt(hAES.hcryp, (uint32_t*) pSrc, Sz, (uint32_t*) pDst, 1000) == HAL_OK);
  unlock();

  return ret;
}

uint8_t AES_Decrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz) {
  uint8_t ret;

  lock();
  ret = (HAL_CRYP_Decrypt(hAES.hcryp, (uint32_t*) pSrc, Sz, (uint32_t*) pDst, 1000) == HAL_OK);
  unlock();

  return ret;
}

void AES_Tester(void) {
  uint8_t Plaintext[] = {
      0x01, 0x02, 0x03, 0x04,
      0xAA, 0xBB, 0xCC, 0xDD,
      0xA9, 0xA8, 0xA7, 0xA6,
      0x50, 0x60, 0x70, 0x80
  };
  uint8_t Encryptedtext[sizeof(Plaintext)] = { 0 };
  uint8_t Decryptedtext[sizeof(Plaintext)] = { 0 };

  // Encrypt & Decrypt
  if (AES_Encrypt(Encryptedtext, Plaintext, sizeof(Plaintext)))
    LOG_StrLn("AES:EncryptOK");

  if (AES_Decrypt(Decryptedtext, Encryptedtext, sizeof(Encryptedtext)))
    LOG_StrLn("AES:DecryptOK");

  // result
  if (memcmp(Plaintext, Decryptedtext, sizeof(Plaintext)) == 0) 
    LOG_StrLn("AES:ItWorks!");
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  osMutexAcquire(AesMutexHandle, osWaitForever);
}

static void unlock(void) {
  osMutexRelease(AesMutexHandle);
}
