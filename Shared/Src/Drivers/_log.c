/*
 * _log.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_log.h"
#include "App/_common.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t LogRecMutexHandle;
#endif

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static void SendITM(char ch);

/* Public functions implementation
 * --------------------------------------------*/
int __io_putchar(int ch) {
  SendITM(ch);
  return ch;
}

int _write(int file, char* ptr, int len) {
  int DataIdx;

  lock();
  for (DataIdx = 0; DataIdx < len; DataIdx++) __io_putchar(*ptr++);
  unlock();

  return len;
}

void printf_init(void) { setvbuf(stdout, NULL, _IONBF, 0); }

void printf_hex(char* data, uint16_t size) {
  lock();
  for (uint32_t i = 0; i < size; i++) printf("%02X", *(data + i));
  unlock();
}

// void Log(const char *fmt, ...) {
//   va_list args;
//   lock();
//   va_start(args, fmt);
//   vprintf(fmt, args);
//   va_end(args);
//   unlock();
// }

/* Private functions implementations
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(LogRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(LogRecMutexHandle);
#endif
}

static void SendITM(char ch) {
#ifdef DEBUG
  uint32_t tick;

  // wait if busy
  tick = _GetTickMS();
  while (_TickIn(tick, LOG_TIMEOUT_MS) && ITM->PORT[0].u32 == 0) {};

  ITM->PORT[0].u8 = (uint8_t)ch;
#endif
}
