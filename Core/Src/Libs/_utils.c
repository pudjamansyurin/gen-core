/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions implementation --------------------------------------------*/
void _DelayMS(uint32_t ms) {
#if RTOS_ENABLE
    osDelay(ms);
#else
    HAL_Delay(ms);
#endif
}

uint32_t _GetTickMS(void) {
#if RTOS_ENABLE
    return osKernelGetTickCount();
#else
    return HAL_GetTick();
#endif
}

uint8_t _LedRead(void) {
    return HAL_GPIO_ReadPin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void _LedWrite(uint8_t state) {
    HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void _LedToggle(void) {
    HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void _Error(char msg[50]) {
#if RTOS_ENABLE
    if (osKernelGetState() == osKernelRunning) {
        LOG_StrLn(msg);
    }
#else
    LOG_StrLn(msg);
#endif

    // indicator error
    while (1) {
        _LedToggle();
        HAL_Delay(50);
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
