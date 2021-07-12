/*
 * log.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/log.h"

#include "App/common.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t LogRecMutexHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define LOG_TIMEOUT_MS ((uint16_t)2)

/* Private functions prototype
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);
static void SendITM(char ch);

/* Public functions implementation
 * --------------------------------------------*/
int __io_putchar(int ch) {
  SendITM(ch);
  return ch;
}

int _write(int file, char* ptr, int len) {
  int DataIdx;

  Lock();
  for (DataIdx = 0; DataIdx < len; DataIdx++) __io_putchar(*ptr++);
  UnLock();

  return len;
}

void printf_init(void) { setvbuf(stdout, NULL, _IONBF, 0); }

void printf_hex(const char* data, uint16_t size) {
  Lock();
  for (uint32_t i = 0; i < size; i++) printf("%02X", *(data + i));
  UnLock();
}

/* Private functions implementations
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(LogRecMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(LogRecMutexHandle);
#endif
}

static void SendITM(char ch) {
#ifdef DEBUG
  uint32_t tick;

  // wait if busy
  tick = tickMs();
  while (tickIn(tick, LOG_TIMEOUT_MS)) {
    if (ITM->PORT[0].u32 != 0) break;
  };

  ITM->PORT[0].u8 = (uint8_t)ch;
#endif
}
