/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_eeprom.h"

#include "Drivers/_at24c.h"

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

/* Public variables
 * --------------------------------------------*/
eeprom_t EEPROM = {0};

/* Private variables
 * --------------------------------------------*/
static uint8_t EE_SZ[VA_MAX];

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static void InitializeSize(void);
static uint16_t Address(EE_VA va);
static uint8_t GetUsedSpace(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t EE_Init(void) {
  uint8_t ok;

  lock();
  printf("EEPROM:Init\n");
  ok = AT24C_Probe();
  if (ok)
    printf("EEPROM:OK\n");
  else
    printf("EEPROM:Error\n");

  InitializeSize();
  IAP_Init();

#if (!APP)
  if (IEEP_VALUE == IEEP_RESET) {
    SIMCon_EE_Write();

    IAP_SetBootMeta(IEEP_OFFSET, IEEP_SET);
  }
#endif

  EEPROM.active = ok;
  unlock();

  return ok;
}

void EE_Refresh(void) {
  if (!_TickIn(EEPROM.tick, EE_CHECK_MS)) {
    EEPROM.tick = _GetTickMS();
    EEPROM.active = AT24C_Probe();
  }
}

uint8_t EE_Cmd(EE_VA va, void* src, void* dst) {
  uint16_t addr = Address(va);
  uint8_t ok = 1;

  lock();
  if (src != NULL) ok = AT24C_Write(addr, src, EE_SZ[va]);

  if (ok)
    ok = AT24C_Read(addr, dst, EE_SZ[va]);
  else
    memcpy(dst, src, EE_SZ[va]);

  EEPROM.active = ok;
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

static void InitializeSize(void) {
  EE_SZ[VA_AES_KEY] = 4 * sizeof(uint32_t);
  EE_SZ[VA_IAP_VERSION] = sizeof(uint16_t);
  EE_SZ[VA_IAP_FLAG] = sizeof(uint32_t);
  EE_SZ[VA_IAP_TYPE] = sizeof(uint32_t);
  EE_SZ[VA_TRIP_A] = sizeof(uint16_t);
  EE_SZ[VA_TRIP_B] = sizeof(uint16_t);
  EE_SZ[VA_TRIP_ODO] = sizeof(uint16_t);
  EE_SZ[VA_MODE_DRIVE] = sizeof(uint8_t);
  EE_SZ[VA_MODE_TRIP] = sizeof(uint8_t);
  EE_SZ[VA_MODE_AVG] = sizeof(uint8_t);
  EE_SZ[VA_MODE] = sizeof(uint8_t);
  EE_SZ[VA_APN_NAME] = EE_STR_MAX;
  EE_SZ[VA_APN_USER] = EE_STR_MAX;
  EE_SZ[VA_APN_PASS] = EE_STR_MAX;
  EE_SZ[VA_FTP_HOST] = EE_STR_MAX;
  EE_SZ[VA_FTP_USER] = EE_STR_MAX;
  EE_SZ[VA_FTP_PASS] = EE_STR_MAX;
  EE_SZ[VA_MQTT_HOST] = EE_STR_MAX;
  EE_SZ[VA_MQTT_PORT] = EE_STR_MAX;
  EE_SZ[VA_MQTT_USER] = EE_STR_MAX;
  EE_SZ[VA_MQTT_PASS] = EE_STR_MAX;

  EEPROM.used = GetUsedSpace();
}

static uint8_t GetUsedSpace(void) {
  const uint16_t capacity = 4096;
  uint16_t used = 0;

  for (uint8_t v = 0; v < VA_MAX; v++) used += EE_SZ[v];

  return (used * 100) / capacity;
}

static uint16_t Address(EE_VA va) {
  uint16_t reg = 0;

  for (uint8_t v = 0; v < VA_MAX; v++) {
    if (v == va) break;
    reg += EE_SZ[v];
  }

  return reg;
}
