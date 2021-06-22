/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes
 * --------------------------------------------*/
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

uint8_t _TickOut(uint32_t tick, uint32_t ms) {
  return (tick && (_GetTickMS() - tick) > ms);
}

uint8_t _TickIn(uint32_t tick, uint32_t ms) {
  return (tick && (_GetTickMS() - tick) < ms);
}

void _Error(char msg[50]) {
#if RTOS_ENABLE
  if (osKernelGetState() == osKernelRunning) printf(msg);
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

float _MovAvgFloat(averager_float_t* m, float* buf, uint16_t sz, float val) {
  // Subtract the oldest number from the prev sum, add the new number
  m->sum = m->sum - buf[m->pos] + val;
  // Assign the nextNum to the position in the array
  buf[m->pos] = val;
  // Increment position
  m->pos++;
  if (m->pos >= sz) m->pos = 0;
  // calculate filled array
  if (m->len < sz) m->len++;
  // return the average
  return m->sum / m->len;
}
