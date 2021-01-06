/*
 * _log.c
 *
 *  Created on: Sep 23, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_log.h"

/* External variables ----------------------------------------------------------*/
#if (!BOOTLOADER)
extern osMutexId_t LogMutexHandle;
#endif

/* Private functions declarations ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void WriteChar(char ch);

/* Public functions implementation ---------------------------------------------*/
void LOG_Enter(void) {
	lock();
	WriteChar('\n');
	unlock();
}

void LOG_Int(int32_t num) {
	lock();

	char str[10]; // 10 chars max for INT32_MAX
	int i = 0;
	if (num < 0) {
		WriteChar('-');
		num *= -1;
	}
	do
		str[i++] = num % 10 + '0';
	while ((num /= 10) > 0);
	for (i--; i >= 0; i--)
		WriteChar(str[i]);

	unlock();
}

void LOG_Int0(int32_t num) {
	lock();

	char str[10]; // 10 chars max for INT32_MAX
	int i = 0;
	if (num < 0) {
		WriteChar('-');
		num *= -1;
	}
	if ((num < 10) && (num >= 0))
		WriteChar('0');
	do
		str[i++] = num % 10 + '0';
	while ((num /= 10) > 0);
	for (i--; i >= 0; i--)
		WriteChar(str[i]);

	unlock();
}

void LOG_Hex8(uint8_t num) {
	lock();

	WriteChar(HEX_CHARS[(num >> 4) % 0x10]);
	WriteChar(HEX_CHARS[(num & 0x0f) % 0x10]);

	unlock();
}

void LOG_Hex16(uint16_t num) {
	lock();

	uint8_t i;
	for (i = 12; i > 0; i -= 4)
		WriteChar(HEX_CHARS[(num >> i) % 0x10]);
	WriteChar(HEX_CHARS[(num & 0x0f) % 0x10]);

	unlock();
}

void LOG_Hex32(uint32_t num) {
	lock();

	uint8_t i;
	for (i = 28; i > 0; i -= 4)
		WriteChar(HEX_CHARS[(num >> i) % 0x10]);
	WriteChar(HEX_CHARS[(num & 0x0f) % 0x10]);

	unlock();
}

void LOG_Str(char *str) {
	lock();

	while (*str != '\0')
		WriteChar(*str++);

	unlock();
}

void LOG_StrLn(char *str) {
	lock();

	while (*str != '\0') {
		WriteChar(*str++);
	}
	WriteChar('\n');

	unlock();
}

void LOG_Buf(char *buf, uint16_t bufsize) {
	lock();

	uint16_t i;
	for (i = 0; i < bufsize; i++)
		WriteChar(*buf++);

	unlock();
}

void LOG_BufPrintable(char *buf, uint16_t bufsize, char subst) {
	lock();

	uint16_t i;
	char ch;
	for (i = 0; i < bufsize; i++) {
		ch = *buf++;
		WriteChar(ch > 32 ? ch : subst);
	}

	unlock();
}

void LOG_BufHex(char *buf, uint16_t bufsize) {
	lock();

	uint16_t i;
	char ch;
	for (i = 0; i < bufsize; i++) {
		ch = *buf++;
		WriteChar(HEX_CHARS[(ch >> 4) % 0x10]);
		WriteChar(HEX_CHARS[(ch & 0x0f) % 0x10]);
	}

	unlock();
}

void LOG_BufHexFancy(char *buf, uint16_t bufsize, uint8_t column_width, char subst) {
	lock();

	uint16_t i = 0, len, pos;
	char buffer[column_width];

	while (i < bufsize) {
		// Line number
		LOG_Hex16(i);
		WriteChar(':');
		WriteChar(' '); // Faster and less code than LOG_Str(": ");

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
		WriteChar(' ');

		// Raw data
		pos = 0;
		do
			WriteChar(buffer[pos] > 32 ? buffer[pos] : subst);
		while (++pos < len);
		WriteChar('\n');

		i += len;
	}

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
