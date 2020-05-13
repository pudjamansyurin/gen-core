/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Libs/_handlebar.h"
#include "Nodes/VCU.h"

/* External variables ----------------------------------------------------------*/
extern vcu_t VCU;
extern sw_t SW;

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
	// indicator error
	while (1) {
		_LedToggle();
		HAL_Delay(50);
	}
}

void _RTOS_Debugger(uint32_t ms) {
	static uint8_t init = 1, thCount;
	static TickType_t lastDebug;
	static osThreadId_t threads[20];
	static uint32_t thLowestFreeStack[20];
	uint32_t thFreeStack;
	char thName[15];

	// Initialize
	if (init) {
		init = 0;
		lastDebug = osKernelGetTickCount();

		// Get list of active threads
		thCount = osThreadGetCount();
		osThreadEnumerate(threads, thCount);

		// Fill initial free stack
		for (uint8_t i = 0; i < thCount; i++) {
			if (threads[i] != NULL) {
				thLowestFreeStack[i] = osThreadGetStackSpace(threads[i]);
			}
		}
	}

	if ((osKernelGetTickCount() - lastDebug) >= pdMS_TO_TICKS(ms)) {
		lastDebug = osKernelGetTickCount();

		// Debug each thread's Stack Space
		LOG_StrLn("============ LOWEST FREE STACK ============");
		for (uint8_t i = 0; i < thCount; i++) {
			if (threads[i] != NULL) {
				// check stack used
				thFreeStack = osThreadGetStackSpace(threads[i]);
				if (thFreeStack < thLowestFreeStack[i]) {
					thLowestFreeStack[i] = thFreeStack;
				}

				// print
				sprintf(thName, "%-14s", osThreadGetName(threads[i]));
				LOG_Buf(thName, strlen(thName));
				LOG_Str(" : ");
				LOG_Int(thLowestFreeStack[i]);
				LOG_StrLn(" Words");
			}
		}
		LOG_StrLn("===========================================");
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

void _DummyGenerator(void) {
	uint8_t *pRange = &(SW.runner.mode.sub.report[SW_M_REPORT_RANGE]);
	uint8_t *pEfficiency = &(SW.runner.mode.sub.report[SW_M_REPORT_EFFICIENCY]);

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

uint32_t _ByteSwap32(uint32_t x) {
	uint32_t y = (x >> 24) & 0xff;
	y |= ((x >> 16) & 0xff) << 8;
	y |= ((x >> 8) & 0xff) << 16;
	y |= (x & 0xff) << 24;

	return y;
}

