/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"
#include "_rtc.h"
#include "VCU.h"

/* External variables ----------------------------------------------------------*/
extern vcu_t VCU;

/* Public functions implementation --------------------------------------------*/
uint8_t _LedRead(void) {
	return HAL_GPIO_ReadPin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void _LedWrite(uint8_t state) {
	HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void _LedToggle(void) {
	HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void _LedDisco(uint16_t ms) {
	uint8_t i = 20, temp;
	uint16_t delay = ms / i;

	// preserve previous state
	temp = _LedRead();

	// run the disco
	while (i--) {
		_LedWrite(i % 2);
		osDelay(delay);
	}

	// restore previous state
	_LedWrite(temp);
}

void _Error(char msg[50]) {
	if (osKernelGetState() == osKernelRunning) {
		LOG_StrLn(msg);
	}
	while (1) {
		_LedToggle();
		HAL_Delay(50);
	}
}

void _RTOS_Debugger(void) {
	static uint8_t init = 1, thCount;
	static TickType_t lastDebug;
	static osThreadId_t threads[20];
	char thName[15];

	// Initialize
	if (init) {
		init = 0;
		lastDebug = osKernelGetTickCount();

		// Get list of active threads
		thCount = osThreadGetCount();
		osThreadEnumerate(threads, thCount);
	}

	if ((osKernelGetTickCount() - lastDebug) >= pdMS_TO_TICKS(10000)) {
		lastDebug = osKernelGetTickCount();

		// Debug each thread's Stack Space
		LOG_StrLn("============ STACK MONITOR ============");
		for (uint8_t i = 0; i < thCount; i++) {
			if (threads[i] != NULL) {
				sprintf(thName, "%-14s", osThreadGetName(threads[i]));
				// print
				LOG_Buf(thName, strlen(thName));
				LOG_Str(" : ");
				LOG_Int(osThreadGetStackSpace(threads[i]));
				LOG_StrLn(" Words");
			}
		}
		LOG_StrLn("=======================================");
	}
}

uint8_t _RTOS_ValidThreadFlag(uint32_t flag) {
	uint8_t ret = 1;

	// check is empty
	if (!flag) {
		ret = 0;
	} else if (flag & (~EVT_MASK)) {
		// error
		ret = 0;
	}

	return ret;
}

uint8_t _RTOS_ValidEventFlag(uint32_t flag) {
	uint8_t ret = 1;

	// check is empty
	if (!flag) {
		ret = 0;
	} else if (flag & (~EVENT_MASK)) {
		// error
		ret = 0;
	}

	return ret;
}

void _DummyGenerator(sw_t *sw) {
	uint8_t *pRange = &(sw->runner.mode.sub.report[SW_M_REPORT_RANGE]);
	uint8_t *pEfficiency = &(sw->runner.mode.sub.report[SW_M_REPORT_EFFICIENCY]);

	// Dummy Report Range
	if (!(*pRange)) {
		*pRange = 255;
	} else {
		(*pRange)--;
	}

	// Dummy Report Efficiency
	if (*pEfficiency >= 255) {
		*pEfficiency = 0;
	} else {
		(*pEfficiency)++;
	}
}

uint8_t _TimeCheckDaylight(timestamp_t timestamp) {
	return (timestamp.time.Hours >= 5 && timestamp.time.Hours <= 16);
}

uint8_t _TimeNeedCalibration(void) {
	// Retrieve RTC time
	RTC_ReadRaw(&(VCU.d.rtc.timestamp));

	// Compare
	return (VCU.d.rtc.calibration.Year != VCU.d.rtc.timestamp.date.Year ||
			VCU.d.rtc.calibration.Month != VCU.d.rtc.timestamp.date.Month ||
			VCU.d.rtc.calibration.Date != VCU.d.rtc.timestamp.date.Date);
}

int8_t _BitPosition(uint64_t event_id) {
	uint8_t pos = -1;

	for (int8_t i = 0; i < 64; i++) {
		if (event_id & BIT(i)) {
			pos = i;
			break;
		}
	}

	return pos;
}

int32_t _ParseNumber(const char *ptr, uint8_t *cnt) {
	uint8_t minus = 0, i = 0;
	int32_t sum = 0;

	if (*ptr == '-') { /* Check for minus character */
		minus = 1;
		ptr++;
		i++;
	}
	while (CHARISNUM(*ptr)) { /* Parse number */
		sum = 10 * sum + CHARTONUM(*ptr);
		ptr++;
		i++;
	}
	if (cnt != NULL) { /* Save number of characters used for number */
		*cnt = i;
	}
	if (minus) { /* Minus detected */
		return 0 - sum;
	}
	return sum; /* Return number */
}

float _ParseFloatNumber(const char *ptr, uint8_t *cnt) {
	uint8_t i = 0, j = 0;
	float sum = 0.0f;

	sum = (float) _ParseNumber(ptr, &i); /* Parse number */
	j += i;
	ptr += i;
	if (*ptr == '.') { /* Check decimals */
		float dec;
		dec = (float) _ParseNumber(ptr + 1, &i);
		dec /= (float) pow(10, i);
		if (sum >= 0) {
			sum += dec;
		} else {
			sum -= dec;
		}
		j += i + 1;
	}

	if (cnt != NULL) { /* Save number of characters used for number */
		*cnt = j;
	}
	return sum; /* Return number */
}

uint32_t _ByteSwap32(uint32_t x) {
	uint32_t y = (x >> 24) & 0xff;
	y |= ((x >> 16) & 0xff) << 8;
	y |= ((x >> 8) & 0xff) << 16;
	y |= (x & 0xff) << 24;

	return y;
}

