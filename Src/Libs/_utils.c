/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"
#include "_rtc.h"

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
	} else {
		while (1) {
			_LedToggle();
			HAL_Delay(50);
		}
	}
}

void _DebugTask(char name[20]) {
	//  LOG_Str("Task:");
	//  LOG_Buf(name, strlen(name));
	//  LOG_Enter();
}

void _DebugStackSpace(osThreadId_t *threads, uint8_t count) {
	char thName[20], thStateString[15];
	osThreadState_t thState;

	// Debug each thread's Stack Space
	LOG_StrLn("============ STACK MONITOR ============");
	for (uint8_t i = 0; i < count; i++) {
		if (threads[i] != NULL) {
			sprintf(thName, "%-19s", osThreadGetName(threads[i]));
			thState = osThreadGetState(threads[i]);

			switch (thState) {
				case osThreadInactive:
					sprintf(thStateString, "%-14s", "Inactive");
					break;
				case osThreadReady:
					sprintf(thStateString, "%-14s", "Ready");
					break;
				case osThreadRunning:
					sprintf(thStateString, "%-14s", "Running");
					break;
				case osThreadBlocked:
					sprintf(thStateString, "%-14s", "Blocked");
					break;
				case osThreadTerminated:
					sprintf(thStateString, "%-14s", "Terminated");
					break;
				case osThreadError:
					sprintf(thStateString, "%-14s", "Error");
					break;
				case osThreadReserved:
					sprintf(thStateString, "%-14s", "Reserved");
					break;
				default:
					sprintf(thStateString, "%-14s", "Unknown");
					break;
			}
			// print
			LOG_Buf(thName, strlen(thName));
			LOG_Str(" (");
			LOG_Buf(thStateString, strlen(thStateString));
			LOG_Str(") : ");
			LOG_Int(osThreadGetStackSpace(threads[i]));
			LOG_StrLn(" Byte");
		}
	}
	LOG_StrLn("=======================================");
}

void _DummyGenerator(db_t *db, sw_t *sw) {
	//  // Dummy algorithm
	//  db->vcu.odometer = (db->vcu.odometer >= VCU_ODOMETER_MAX ? 0 : (db->vcu.odometer + 1));

	// Dummy Report Range
	if (!sw->runner.mode.sub.report[SW_M_REPORT_RANGE]) {
		sw->runner.mode.sub.report[SW_M_REPORT_RANGE] = 255;
	} else {
		sw->runner.mode.sub.report[SW_M_REPORT_RANGE]--;
	}
	// Dummy Report Efficiency
	if (sw->runner.mode.sub.report[SW_M_REPORT_EFFICIENCY] >= 255) {
		sw->runner.mode.sub.report[SW_M_REPORT_EFFICIENCY] = 0;
	} else {
		sw->runner.mode.sub.report[SW_M_REPORT_EFFICIENCY]++;
	}

}

uint8_t _TimeCheckDaylight(timestamp_t timestamp) {
	return (timestamp.time.Hours >= 5 && timestamp.time.Hours <= 16);
}

uint8_t _TimeNeedCalibration(db_t *db) {
	// Retrieve RTC time
	RTC_ReadRaw(&(db->vcu.rtc.timestamp));

	// Compare
	return (db->vcu.rtc.calibration.Year != db->vcu.rtc.timestamp.date.Year ||
			db->vcu.rtc.calibration.Month != db->vcu.rtc.timestamp.date.Month ||
			db->vcu.rtc.calibration.Date != db->vcu.rtc.timestamp.date.Date);
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

