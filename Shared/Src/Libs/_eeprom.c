/*
 * _eeprom.c
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

/* Private constants
 * --------------------------------------------*/
#define EE_CHECK_MS (60000)
#define EE_CAPACITY_B (4096)

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint8_t active;
  uint8_t used;
  uint32_t tick;
  uint8_t size[VA_MAX];
} eeprom_t;

/* Private variables
 * --------------------------------------------*/
static eeprom_t EEPROM = {0};

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

uint8_t EE_Cmd(EE_VA va, const void* src, void* dst) {
  uint16_t addr = Address(va);
  uint8_t ok = 1;

  lock();
  if (src != NULL) ok = AT24C_Write(addr, src, EEPROM.size[va]);

  if (ok)
    ok = AT24C_Read(addr, dst, EEPROM.size[va]);
  else
    memcpy(dst, src, EEPROM.size[va]);

  EEPROM.active = ok;
  unlock();

  return ok;
}

uint8_t EE_IO_Active(void) {
	return EEPROM.active;
}

uint8_t EE_IO_Used(void) {
	return EEPROM.used;
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
  EEPROM.size[VA_AES_KEY] = 4 * sizeof(uint32_t);
  EEPROM.size[VA_IAP_VERSION] = sizeof(uint16_t);
  EEPROM.size[VA_IAP_FLAG] = sizeof(uint32_t);
  EEPROM.size[VA_IAP_TYPE] = sizeof(uint32_t);
  EEPROM.size[VA_TRIP_A] = sizeof(uint16_t);
  EEPROM.size[VA_TRIP_B] = sizeof(uint16_t);
  EEPROM.size[VA_TRIP_ODO] = sizeof(uint16_t);
  EEPROM.size[VA_MODE_DRIVE] = sizeof(uint8_t);
  EEPROM.size[VA_MODE_TRIP] = sizeof(uint8_t);
  EEPROM.size[VA_MODE_AVG] = sizeof(uint8_t);
  EEPROM.size[VA_MODE] = sizeof(uint8_t);
  EEPROM.size[VA_APN_NAME] = EE_STR_MAX;
  EEPROM.size[VA_APN_USER] = EE_STR_MAX;
  EEPROM.size[VA_APN_PASS] = EE_STR_MAX;
  EEPROM.size[VA_FTP_HOST] = EE_STR_MAX;
  EEPROM.size[VA_FTP_USER] = EE_STR_MAX;
  EEPROM.size[VA_FTP_PASS] = EE_STR_MAX;
  EEPROM.size[VA_MQTT_HOST] = EE_STR_MAX;
  EEPROM.size[VA_MQTT_PORT] = EE_STR_MAX;
  EEPROM.size[VA_MQTT_USER] = EE_STR_MAX;
  EEPROM.size[VA_MQTT_PASS] = EE_STR_MAX;

  EEPROM.used = GetUsedSpace();
}

static uint8_t GetUsedSpace(void) {
  uint16_t used = 0;

  for (uint8_t v = 0; v < VA_MAX; v++) used += EEPROM.size[v];

  return (used * 100) / EE_CAPACITY_B;
}

static uint16_t Address(EE_VA va) {
  uint16_t reg = 0;

  for (uint8_t v = 0; v < VA_MAX; v++) {
    if (v == va) break;
    reg += EEPROM.size[v];
  }

  return reg;
}
