/*
 * _log.c
 *
 *  Created on: Sep 23, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_log.h"

/* External variables ----------------------------------------------------------*/
extern osMutexId LogMutexHandle;

/* Public functions implementation ---------------------------------------------*/
void LOG_Char(char ch) {
#if LOG_DEBUG
  // wait if busy
  while (ITM->PORT[0].u32 == 0)
    ;
  // send to ITM0
  ITM->PORT[0].u8 = (uint8_t) ch;
#endif
}

void LOG_Enter(void) {
  osMutexWait(LogMutexHandle, osWaitForever);
  LOG_Char('\n');
  osMutexRelease(LogMutexHandle);
}

void LOG_Int(int32_t num) {
  osMutexWait(LogMutexHandle, osWaitForever);

  char str[10]; // 10 chars max for INT32_MAX
  int i = 0;
  if (num < 0) {
    LOG_Char('-');
    num *= -1;
  }
  do
    str[i++] = num % 10 + '0';
  while ((num /= 10) > 0);
  for (i--; i >= 0; i--)
    LOG_Char(str[i]);

  osMutexRelease(LogMutexHandle);
}

void LOG_Int0(int32_t num) {
  osMutexWait(LogMutexHandle, osWaitForever);

  char str[10]; // 10 chars max for INT32_MAX
  int i = 0;
  if (num < 0) {
    LOG_Char('-');
    num *= -1;
  }
  if ((num < 10) && (num >= 0))
    LOG_Char('0');
  do
    str[i++] = num % 10 + '0';
  while ((num /= 10) > 0);
  for (i--; i >= 0; i--)
    LOG_Char(str[i]);

  osMutexRelease(LogMutexHandle);
}

void LOG_Hex8(uint8_t num) {
  osMutexWait(LogMutexHandle, osWaitForever);

  LOG_Char(HEX_CHARS[(num >> 4) % 0x10]);
  LOG_Char(HEX_CHARS[(num & 0x0f) % 0x10]);

  osMutexRelease(LogMutexHandle);
}

void LOG_Hex16(uint16_t num) {
  osMutexWait(LogMutexHandle, osWaitForever);

  uint8_t i;
  for (i = 12; i > 0; i -= 4)
    LOG_Char(HEX_CHARS[(num >> i) % 0x10]);
  LOG_Char(HEX_CHARS[(num & 0x0f) % 0x10]);

  osMutexRelease(LogMutexHandle);
}

void LOG_Hex32(uint32_t num) {
  osMutexWait(LogMutexHandle, osWaitForever);

  uint8_t i;
  for (i = 28; i > 0; i -= 4)
    LOG_Char(HEX_CHARS[(num >> i) % 0x10]);
  LOG_Char(HEX_CHARS[(num & 0x0f) % 0x10]);

  osMutexRelease(LogMutexHandle);
}

void LOG_Str(char *str) {
  osMutexWait(LogMutexHandle, osWaitForever);

  while (*str != '\0')
    LOG_Char(*str++);

  osMutexRelease(LogMutexHandle);
}

void LOG_StrLn(char *str) {
  osMutexWait(LogMutexHandle, osWaitForever);

  while (*str != '\0')
    LOG_Char(*str++);
  LOG_Char('\n');

  osMutexRelease(LogMutexHandle);
}

void LOG_Buf(char *buf, uint16_t bufsize) {
  osMutexWait(LogMutexHandle, osWaitForever);

  uint16_t i;
  for (i = 0; i < bufsize; i++)
    LOG_Char(*buf++);

  osMutexRelease(LogMutexHandle);
}

void LOG_BufPrintable(char *buf, uint16_t bufsize, char subst) {
  osMutexWait(LogMutexHandle, osWaitForever);

  uint16_t i;
  char ch;
  for (i = 0; i < bufsize; i++) {
    ch = *buf++;
    LOG_Char(ch > 32 ? ch : subst);
  }

  osMutexRelease(LogMutexHandle);
}

void LOG_BufHex(char *buf, uint16_t bufsize) {
  osMutexWait(LogMutexHandle, osWaitForever);

  uint16_t i;
  char ch;
  for (i = 0; i < bufsize; i++) {
    ch = *buf++;
    LOG_Char(HEX_CHARS[(ch >> 4) % 0x10]);
    LOG_Char(HEX_CHARS[(ch & 0x0f) % 0x10]);
  }

  osMutexRelease(LogMutexHandle);
}

void LOG_BufHexFancy(char *buf, uint16_t bufsize, uint8_t column_width, char subst) {
  osMutexWait(LogMutexHandle, osWaitForever);

  uint16_t i = 0, len, pos;
  char buffer[column_width];

  while (i < bufsize) {
    // Line number
    LOG_Hex16(i);
    LOG_Char(':');
    LOG_Char(' '); // Faster and less code than LOG_Str(": ");

    // Copy one line
    if (i + column_width >= bufsize)
      len = bufsize - i;
    else
      len = column_width;
    memcpy(buffer, &buf[i], len);

    // Hex data
    pos = 0;
    while (pos < len)
      LOG_Hex8(buffer[pos++]);
    LOG_Char(' ');

    // Raw data
    pos = 0;
    do
      LOG_Char(buffer[pos] > 32 ? buffer[pos] : subst);
    while (++pos < len);
    LOG_Char('\n');

    i += len;
  }

  osMutexRelease(LogMutexHandle);
}
