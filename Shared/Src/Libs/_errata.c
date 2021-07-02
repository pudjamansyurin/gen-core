/*
 * _errata.c
 *
 *  Created on: Jun 30, 2021
 *      Author: pujak
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_errata.h"

#include "i2c.h"

/* Private functions prototype
 * --------------------------------------------*/
static void ResetClockOnly(void);
//static void ResetComplete(void);
//static void ClearBusyFlagErrata_2_14_7(i2c_errata_t *h);

/* Public functions implementation
 * --------------------------------------------*/
void ERRATA_I2C_ClearBusyFlag(void) {
	ResetClockOnly();
//	ResetComplete();
}

/* Private functions implementation
 * --------------------------------------------*/
static void ResetClockOnly(void) {
	__HAL_RCC_I2C2_CLK_ENABLE();
#if (APP)
	__HAL_RCC_I2C1_CLK_ENABLE();
	__HAL_RCC_I2C3_CLK_ENABLE();
#endif
	HAL_Delay(100);

	__HAL_RCC_I2C2_FORCE_RESET();
#if (APP)
	__HAL_RCC_I2C1_FORCE_RESET();
	__HAL_RCC_I2C3_FORCE_RESET();
#endif
	HAL_Delay(100);

	__HAL_RCC_I2C2_RELEASE_RESET();
#if (APP)
	__HAL_RCC_I2C1_RELEASE_RESET();
	__HAL_RCC_I2C3_RELEASE_RESET();
#endif
	HAL_Delay(100);
}

//static void ResetComplete(void) {
//	hi2c2.Instance = I2C2;
//	i2c_errata_t i2c2 = {
//			.hi2c = &hi2c2,
//			.sda = {
//					.gpio = INT_EEPROM_SDA_GPIO_Port,
//					.alt = GPIO_AF9_I2C2,
//					.pin = INT_EEPROM_SDA_Pin,
//			},
//			.scl = {
//					.gpio = INT_EEPROM_SCL_GPIO_Port,
//					.alt = GPIO_AF4_I2C2,
//					.pin = INT_EEPROM_SCL_Pin,
//			},
//	};
//
//	ClearBusyFlagErrata_2_14_7(&i2c2);
////#if APP
////	ClearBusyFlagErrata_2_14_7(&i2c1);
////	ClearBusyFlagErrata_2_14_7(&i2c3);
////#endif
//}
//
//
//static void ClearBusyFlagErrata_2_14_7(i2c_errata_t *h) {
//	GPIO_InitTypeDef GPIO_InitStruct;
//
//	// 1 Disable the I2C peripheral by clearing the PE bit in I2Cx_CR1 register.
//	__HAL_I2C_DISABLE(h->hi2c);
//
//	// 2 Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//	GPIO_InitStruct.Pull = GPIO_PULLUP;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
//
//	GPIO_InitStruct.Pin = h->scl.pin;
//	HAL_GPIO_Init(h->scl.gpio, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(h->scl.gpio, h->scl.pin, GPIO_PIN_SET);
//
//	GPIO_InitStruct.Pin = h->sda.pin;
//	HAL_GPIO_Init(h->sda.gpio, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(h->sda.gpio, h->sda.pin, GPIO_PIN_SET);
//
//	// 3 Check SCL and SDA High level in GPIOx_IDR.
//	while(HAL_GPIO_ReadPin(h->sda.gpio, h->sda.pin) != GPIO_PIN_SET) {
//		HAL_Delay(1);
//	}
//	while (HAL_GPIO_ReadPin(h->scl.gpio, h->scl.pin) != GPIO_PIN_SET) {
//        //Move clock to release I2C
//        HAL_GPIO_WritePin(h->scl.gpio, h->scl.pin, GPIO_PIN_RESET);
//		HAL_Delay(1);
//        HAL_GPIO_WritePin(h->scl.gpio, h->scl.pin, GPIO_PIN_SET);
//		HAL_Delay(1);
//	}
//
//	// 4 Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
//	// GPIO_InitStruct.Pin = h->sda.pin;
//	// HAL_GPIO_Init(h->sda.gpio, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(h->sda.gpio, h->sda.pin, GPIO_PIN_RESET);
//
//	// 5 Check SDA Low level in GPIOx_IDR.
//	while (HAL_GPIO_ReadPin(h->sda.gpio, h->sda.pin) != GPIO_PIN_RESET) {
//		HAL_Delay(1);
//	}
//
//	// 6 Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
//	//	GPIO_InitStruct.Pin = h->scl.pin;
//	//	HAL_GPIO_Init(h->scl.gpio, &GPIO_InitStruct);
//	//	HAL_GPIO_TogglePin(h->scl.gpio, h->scl.pin);
//	HAL_GPIO_WritePin(h->scl.gpio, h->scl.pin, GPIO_PIN_RESET);
//
//	// 7 Check SCL Low level in GPIOx_IDR.
//	while (HAL_GPIO_ReadPin(h->scl.gpio, h->scl.pin) != GPIO_PIN_RESET) {
//		HAL_Delay(1);
//	}
//
//	// 8 Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
//	//	GPIO_InitStruct.Pin = h->scl.pin;
//	//	HAL_GPIO_Init(h->scl.gpio, &GPIO_InitStruct);
//	//	GPIO_WRITE_ODR(h->scl.gpio, h->scl.pin);
//	HAL_GPIO_WritePin(h->scl.gpio, h->scl.pin, GPIO_PIN_SET);
//
//	// 9 Check SCL High level in GPIOx_IDR.
//	while (HAL_GPIO_ReadPin(h->scl.gpio, h->scl.pin) != GPIO_PIN_SET) {
//		HAL_Delay(1);
//	}
//
//	// 10 Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
//	//	GPIO_InitStruct.Pin = h->sda.pin;
//	//	HAL_GPIO_Init(h->sda.gpio, &GPIO_InitStruct);
//	//	GPIO_WRITE_ODR(h->sda.gpio, h->sda.pin);
//	HAL_GPIO_WritePin(h->sda.gpio, h->sda.pin, GPIO_PIN_SET);
//
//	// 11 Check SDA High level in GPIOx_IDR.
//	while (HAL_GPIO_ReadPin(h->sda.gpio, h->sda.pin) != GPIO_PIN_SET) {
//		HAL_Delay(1);
//	}
//
//	// 12  Configure the SCL and SDA I/Os as Alternate function Open-Drain.
//	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//	GPIO_InitStruct.Pull = GPIO_PULLUP;
//
//	GPIO_InitStruct.Pin = h->scl.pin;
//	GPIO_InitStruct.Alternate = h->scl.alt;
//	HAL_GPIO_Init(h->scl.gpio, &GPIO_InitStruct);
//
//	GPIO_InitStruct.Pin = h->sda.pin;
//	GPIO_InitStruct.Alternate = h->sda.alt;
//	HAL_GPIO_Init(h->sda.gpio, &GPIO_InitStruct);
//
//	HAL_GPIO_WritePin(h->scl.gpio, h->scl.pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(h->sda.gpio, h->sda.pin, GPIO_PIN_SET);
//
//	// 13 Set SWRST bit in I2Cx_CR1 register.
//	h->hi2c->Instance->CR1 |= I2C_CR1_SWRST;
//
//	// 14 Clear SWRST bit in I2Cx_CR1 register.
//	h->hi2c->Instance->CR1 ^= I2C_CR1_SWRST;
//
//	// 15 Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register.
//	__HAL_I2C_ENABLE(h->hi2c);
//}
