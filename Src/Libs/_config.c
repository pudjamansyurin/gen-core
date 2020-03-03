/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#include <math.h>
#include "_config.h"
#include "_database.h"

void BSP_LedWrite(uint8_t state) {
	HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void BSP_LedToggle(void) {
	HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void BSP_LedDisco(uint16_t ms) {
	uint8_t i = 100;
	uint16_t delay = ms / i;

	while (i--) {
		BSP_LedToggle();
		osDelay(delay);
	}
}

int8_t BSP_BitPosition(uint64_t event_id) {
	uint8_t pos = -1;

	for (int8_t i = 0; i < 64; i++) {
		if (event_id & SetBit(i)) {
			pos = i;
			break;
		}
	}

	return pos;
}

uint32_t BSP_ChangeEndian32(uint32_t val) {
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
	return (val << 16) | (val >> 16);
}

uint64_t BSP_ChangeEndian64(uint64_t val) {
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return (val << 32) | (val >> 32);
}

int32_t BSP_ParseNumber(const char *ptr, uint8_t *cnt) {
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

float BSP_ParseFloatNumber(const char *ptr, uint8_t *cnt) {
	uint8_t i = 0, j = 0;
	float sum = 0.0f;

	sum = (float) BSP_ParseNumber(ptr, &i); /* Parse number */
	j += i;
	ptr += i;
	if (*ptr == '.') { /* Check decimals */
		float dec;
		dec = (float) BSP_ParseNumber(ptr + 1, &i);
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

//void BSP_Read_Handlebar(switch_t *DB_VCU_Switch, uint8_t DB_VCU_Switch_Size) {
//	uint8_t i;
//
//	for (i = 0; i < DB_VCU_Switch_Size; i++) {
//		DB_VCU_Switch[i].state = HAL_GPIO_ReadPin(DB_VCU_Switch[i].port, DB_VCU_Switch[i].pin);
//	}
//}
