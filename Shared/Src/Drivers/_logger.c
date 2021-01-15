/*
 * _logger.c
 *
 *  Created on: Jan 15, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include "Drivers/_logger.h"
#include "Libs/_utils.h"

/* External variables ----------------------------------------------------------*/
#if (!BOOTLOADER)
extern osMutexId_t LogMutexHandle;
#endif

/* Private functions declarations ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void WriteChar(char ch);

/* Public functions implementation --------------------------------------------*/
int __io_putchar(int ch) {
  WriteChar(ch);
  return ch;
}

//int _write(int32_t file, uint8_t *ptr, int32_t len)
//{
//  int i=0;
//
//  for(i=0 ; i<len ; i++)
//    WriteChar((*ptr++));
//
//  return len;
//}

void LogInit(void) {
  setvbuf(stdout, NULL, _IONBF, 0);
}

void Log(const char *fmt, ...) {
  va_list args;

  lock();
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  unlock();
}

/* Private functions implementations ----------------------------------------------*/
static void lock(void) {
#ifdef DEBUG
#if (!BOOTLOADER)
  osMutexAcquire(LogMutexHandle, osWaitForever);
#endif
#endif
}

static void unlock(void) {
#ifdef DEBUG
#if (!BOOTLOADER)
  osMutexRelease(LogMutexHandle);
#endif
#endif
}

static void WriteChar(char ch) {
#ifdef DEBUG
  uint32_t tick;

  // wait if busy
  tick = _GetTickMS();
  while (_GetTickMS() - tick <= 5) {
    if (ITM->PORT[0].u32 != 0) {
      ITM->PORT[0].u8 = (uint8_t) ch;
      break;
    }
    _DelayMS(1);
  }
#endif
}

