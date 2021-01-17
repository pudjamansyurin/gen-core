/*
 * _log.c
 *
 *  Created on: Jan 15, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include "Drivers/_log.h"
#include "Libs/_utils.h"

/* External variables ----------------------------------------------------------*/
#if (!BOOTLOADER)
extern osMutexId_t LogMutexHandle;
#endif

/* Private functions declarations ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void SendITM(char ch);

/* Public functions implementation --------------------------------------------*/
int __io_putchar(int ch) {
  SendITM(ch);
  return ch;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

  lock();
	for (DataIdx = 0; DataIdx < len; DataIdx++)
		__io_putchar(*ptr++);
  unlock();

	return len;
}

void LogInit(void) {
  setvbuf(stdout, NULL, _IONBF, 0);
}

void LogBufferHex(char *data, uint16_t size) {
  for(uint32_t i=0; i<size; i++)
    printf("%02X", *(data+i));
}
// void Log(const char *fmt, ...) {
//   va_list args;
//   lock();
//   va_start(args, fmt);
//   vprintf(fmt, args);
//   va_end(args);
//   unlock();
// }

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

static void SendITM(char ch) {
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

