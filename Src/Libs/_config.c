/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#include "_config.h"
#include "_database.h"

void BSP_Led_Write(uint8_t state) {
	HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void BSP_Led_Toggle(void) {
	HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void BSP_Led_Disco(uint16_t ms) {
	uint8_t i = 100;
	uint16_t delay = ms / i;

	while (i--) {
		BSP_Led_Toggle();
		osDelay(delay);
	}
}

int8_t BSP_Bit_Position(uint64_t event_id) {
	uint8_t pos = -1;

	for (int8_t i = 0; i < 64; i++) {
		if (event_id & SetBit(i)) {
			pos = i;
			break;
		}
	}

	return pos;
}

uint32_t BSP_Change_Endian32(uint32_t val) {
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

uint64_t BSP_Change_Endian64(uint64_t val) {
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return (val << 32) | (val >> 32);
}

//void BSP_Read_Handlebar(switch_t *DB_VCU_Switch, uint8_t DB_VCU_Switch_Size) {
//	uint8_t i;
//
//	for (i = 0; i < DB_VCU_Switch_Size; i++) {
//		DB_VCU_Switch[i].state = HAL_GPIO_ReadPin(DB_VCU_Switch[i].port, DB_VCU_Switch[i].pin);
//	}
//}
//// FIXME am I needed ?
//// Converts a floating point number to string.
//void ftoa(float f, char *str, char size) {
//	uint8_t pos;  // position in string
//	char len;  // length of decimal part of result
//	char curr[100];  // temp holder for next digit
//	int value;  // decimal digit(s) to convert
//	pos = 0;  // initialize pos, just to be sure
//
//	value = (int) f;  // truncate the floating point number
//	itoa(value, str, 10);  // this is kinda dangerous depending on the length of str
//	// now str array has the digits before the decimal
//
//	if (f < 0)  // handle negative numbers
//			{
//		f *= -1;
//		value *= -1;
//	}
//
//	len = strlen(str);  // find out how big the integer part was
//	pos = len;  // position the pointer to the end of the integer part
//	str[pos++] = '.';  // add decimal point to string
//
//	while (pos < (size + len + 1))  // process remaining digits
//	{
//		f = f - (float) value;  // hack off the whole part of the number
//		f *= 10;  // move next digit over
//		value = (int) f;  // get next digit
//		itoa(value, curr, 10); // convert digit to string
//		str[pos++] = *curr; // add digit to result string and increment pointer
//	}
//}
