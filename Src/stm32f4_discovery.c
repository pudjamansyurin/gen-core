/**
 ******************************************************************************
 * @file    stm32f4_discovery.c
 * @author  MCD Application Team
 * @brief   This file provides set of firmware functions to manage Leds and
 *          push-button available on STM32F4-Discovery Kit from STMicroelectronics.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"

/** @defgroup BSP BSP
 * @{
 */

/** @defgroup STM32F4_DISCOVERY STM32F4 DISCOVERY
 * @{
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL STM32F4 DISCOVERY LOW LEVEL
 * @brief This file provides set of firmware functions to manage Leds and push-button
 *        available on STM32F4-Discovery Kit from STMicroelectronics.
 * @{
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Private_TypesDefinitions STM32F4 DISCOVERY LOW LEVEL Private TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Private_Defines STM32F4 DISCOVERY LOW LEVEL Private Defines
 * @{
 */

/**
 * @brief STM32F4 DISCO BSP Driver version number V2.1.3
 */
/**
 * @}
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Private_Macros STM32F4 DISCOVERY LOW LEVEL Private Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Private_Variables STM32F4 DISCOVERY LOW LEVEL Private Variables
 * @{
 */

uint32_t I2cxTimeout = I2Cx_TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */

static I2C_HandleTypeDef I2cHandle;
/**
 * @}
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Private_FunctionPrototypes STM32F4 DISCOVERY LOW LEVEL Private FunctionPrototypes
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Private_Functions STM32F4 DISCOVERY LOW LEVEL Private Functions
 * @{
 */
static void I2Cx_Init(void);
static void I2Cx_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value);
static uint8_t I2Cx_ReadData(uint8_t Addr, uint8_t Reg);
static void I2Cx_MspInit(void);
static void I2Cx_Error(uint8_t Addr);

/* Link functions for Audio peripheral */
void AUDIO_IO_Init(void);
void AUDIO_IO_DeInit(void);
void AUDIO_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t AUDIO_IO_Read(uint8_t Addr, uint8_t Reg);

/******************************* I2C Routines**********************************/
/**
 * @brief  Configures I2C interface.
 */
static void I2Cx_Init(void) {
	if (HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_RESET) {
		/* DISCOVERY_I2Cx peripheral configuration */
		I2cHandle.Init.ClockSpeed = BSP_I2C_SPEED;
		I2cHandle.Init.DutyCycle = I2C_DUTYCYCLE_2;
		I2cHandle.Init.OwnAddress1 = 0x33;
		I2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		I2cHandle.Instance = DISCOVERY_I2Cx;

		/* Init the I2C */
		I2Cx_MspInit();
		HAL_I2C_Init(&I2cHandle);
	}
}

/**
 * @brief  Write a value in a register of the device through BUS.
 * @param  Addr: Device address on BUS Bus.
 * @param  Reg: The target register address to write
 * @param  Value: The target register value to be written
 * @retval HAL status
 */
static void I2Cx_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Write(&I2cHandle, Addr, (uint16_t) Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2cxTimeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Execute user timeout callback */
		I2Cx_Error(Addr);
	}
}

/**
 * @brief  Read a register of the device through BUS
 * @param  Addr: Device address on BUS
 * @param  Reg: The target register address to read
 * @retval HAL status
 */
static uint8_t I2Cx_ReadData(uint8_t Addr, uint8_t Reg) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t value = 0;

	status = HAL_I2C_Mem_Read(&I2cHandle, Addr, (uint16_t) Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2cxTimeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Execute user timeout callback */
		I2Cx_Error(Addr);
	}
	return value;
}

/**
 * @brief  Manages error callback by re-initializing I2C.
 * @param  Addr: I2C Address
 */
static void I2Cx_Error(uint8_t Addr) {
	/* De-initialize the I2C communication bus */
	HAL_I2C_DeInit(&I2cHandle);

	/* Re-Initialize the I2C communication bus */
	I2Cx_Init();
}

/**
 * @brief I2C MSP Initialization
 */
static void I2Cx_MspInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable I2C GPIO clocks */
	DISCOVERY_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()
	;

	/* DISCOVERY_I2Cx SCL and SDA pins configuration ---------------------------*/
	GPIO_InitStruct.Pin = DISCOVERY_I2Cx_SCL_PIN | DISCOVERY_I2Cx_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Alternate = DISCOVERY_I2Cx_SCL_SDA_AF;
	HAL_GPIO_Init(DISCOVERY_I2Cx_SCL_SDA_GPIO_PORT, &GPIO_InitStruct);

	/* Enable the DISCOVERY_I2Cx peripheral clock */
	DISCOVERY_I2Cx_CLK_ENABLE()
	;

	/* Force the I2C peripheral clock reset */
	DISCOVERY_I2Cx_FORCE_RESET();

	/* Release the I2C peripheral clock reset */
	DISCOVERY_I2Cx_RELEASE_RESET();

	/* Enable and set I2Cx Interrupt to the highest priority */
	HAL_NVIC_SetPriority(DISCOVERY_I2Cx_EV_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DISCOVERY_I2Cx_EV_IRQn);

	/* Enable and set I2Cx Interrupt to the highest priority */
	HAL_NVIC_SetPriority(DISCOVERY_I2Cx_ER_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DISCOVERY_I2Cx_ER_IRQn);
}

/*******************************************************************************
 LINK OPERATIONS
 *******************************************************************************/

/********************************* LINK AUDIO *********************************/

/**
 * @brief  Initializes Audio low level.
 */
void AUDIO_IO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable Reset GPIO Clock */
	AUDIO_RESET_GPIO_CLK_ENABLE()
	;

	/* Audio reset pin configuration */
	GPIO_InitStruct.Pin = AUDIO_RESET_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(AUDIO_RESET_GPIO, &GPIO_InitStruct);

	I2Cx_Init();

	/* Power Down the codec */
	HAL_GPIO_WritePin(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, GPIO_PIN_RESET);

	/* Wait for a delay to insure registers erasing */
	HAL_Delay(5);

	/* Power on the codec */
	HAL_GPIO_WritePin(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, GPIO_PIN_SET);

	/* Wait for a delay to insure registers erasing */
	HAL_Delay(5);
}

/**
 * @brief  DeInitializes Audio low level.
 */
void AUDIO_IO_DeInit(void) {

}

/**
 * @brief  Writes a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  Value: Data to be written
 */
void AUDIO_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value) {
	I2Cx_WriteData(Addr, Reg, Value);
}

/**
 * @brief  Reads a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @retval Data to be read
 */
uint8_t AUDIO_IO_Read(uint8_t Addr, uint8_t Reg) {
	return I2Cx_ReadData(Addr, Reg);
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
