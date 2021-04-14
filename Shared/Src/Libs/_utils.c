/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions implementation
 * --------------------------------------------*/
void _I2C_ClearBusyFlagErratum(void) {
  __HAL_RCC_I2C2_CLK_ENABLE();
#if (!BOOTLOADER)
  __HAL_RCC_I2C1_CLK_ENABLE();
  __HAL_RCC_I2C3_CLK_ENABLE();
#endif
  HAL_Delay(100);

  __HAL_RCC_I2C2_FORCE_RESET();
#if (!BOOTLOADER)
  __HAL_RCC_I2C1_FORCE_RESET();
  __HAL_RCC_I2C3_FORCE_RESET();
#endif
  HAL_Delay(100);

  __HAL_RCC_I2C2_RELEASE_RESET();
#if (!BOOTLOADER)
  __HAL_RCC_I2C1_RELEASE_RESET();
  __HAL_RCC_I2C3_RELEASE_RESET();
#endif
  HAL_Delay(100);
}

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

void _Error(char msg[50]) {
#if RTOS_ENABLE
  if (osKernelGetState() == osKernelRunning)
    printf(msg);
#else
  printf(msg);
#endif
}

uint32_t _ByteSwap32(uint32_t x) {
  uint32_t y = (x >> 24) & 0xff;
  y |= ((x >> 16) & 0xff) << 8;
  y |= ((x >> 8) & 0xff) << 16;
  y |= (x & 0xff) << 24;

  return y;
}

// int8_t _BitPos(uint64_t event_id) {
//  int8_t pos = -1;
//
//  for (int8_t i = 0; i < 64; i++)
//    if (event_id & BIT(i)) {
//      pos = i;
//      break;
//    }
//
//  return pos;
//}
