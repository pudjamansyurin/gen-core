/*
 * _errata.c
 *
 *  Created on: Feb 11, 2021
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_errata.h"

/* Public functions implementation
 * ---------------------------------------------*/
void I2C_ClearBusyFlagErratum1(void) {
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

void I2C_ClearBusyFlagErratum2(I2C_module_t *i2c) {
  GPIO_InitTypeDef GPIO_InitStructure;

  // 1. Clear PE bit.
  i2c->instance.Instance->CR1 &= ~(0x0001);

  //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain,
  //  High level (Write 1 to GPIOx_ODR).
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStructure.Alternate = GPIO_AF4_I2C2;
  GPIO_InitStructure.Pin = i2c->sclPin;
  HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);
  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

  GPIO_InitStructure.Alternate = GPIO_AF9_I2C2;
  GPIO_InitStructure.Pin = i2c->sdaPin;
  HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);
  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);

  // 3. Check SCL and SDA High level in GPIOx_IDR.
  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin)) {
    asm("nop");
  }

  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin)) {
    asm("nop");
  }

  // 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level
  // (Write 0 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_RESET);

  //  5. Check SDA Low level in GPIOx_IDR.
  while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin)) {
    asm("nop");
  }

  // 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level
  // (Write 0 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET);

  //  7. Check SCL Low level in GPIOx_IDR.
  while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin)) {
    asm("nop");
  }

  // 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level
  // (Write 1 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

  // 9. Check SCL High level in GPIOx_IDR.
  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin)) {
    asm("nop");
  }

  // 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level
  // (Write 1 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);

  // 11. Check SDA High level in GPIOx_IDR.
  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin)) {
    asm("nop");
  }

  // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
  GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;

  GPIO_InitStructure.Alternate = GPIO_AF4_I2C2;
  GPIO_InitStructure.Pin = i2c->sclPin;
  HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);

  GPIO_InitStructure.Alternate = GPIO_AF9_I2C2;
  GPIO_InitStructure.Pin = i2c->sdaPin;
  HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);

  // 13. Set SWRST bit in I2Cx_CR1 register.
  i2c->instance.Instance->CR1 |= 0x8000;

  asm("nop");

  // 14. Clear SWRST bit in I2Cx_CR1 register.
  i2c->instance.Instance->CR1 &= ~0x8000;

  asm("nop");

  // 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
  i2c->instance.Instance->CR1 |= 0x0001;

  // Call initialization function.
  HAL_I2C_Init(&(i2c->instance));
}
