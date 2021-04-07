/*
 * _codec.c
 *
 *  Created on: Mar 8, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_codec.h"
#include "i2c.h"

/* Private variables
 * ----------------------------------------------------------*/
static I2C_HandleTypeDef *pi2c = &hi2c1;

/* Private functions prototype ------------------------------------------------*/
static void I2Cx_Error(uint8_t Addr);
static uint8_t CODEC_Check(uint8_t Addr);

/* Public functions implementations ---------------------------------------------*/
/**
 * @brief  Initializes Audio low level.
 */
uint8_t CODEC_Init(uint8_t Addr) {
	uint8_t ok;

	MX_I2C1_Init();
	GATE_AudioCodecReset();

	ok = CODEC_Check(Addr);

	return ok;
}

/**
 * @brief  DeInitializes Audio low level.
 */
void CODEC_DeInit(void) {
	GATE_AudioCodecStop();
	HAL_I2C_DeInit(pi2c);
}

/**
 * @brief  Writes a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  Value: Data to be written
 */
void CODEC_Write(uint8_t Addr, uint8_t Reg, uint8_t Value) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Write(pi2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT,
			&Value, 1, I2Cx_TIMEOUT_MAX);

	/* Check the communication status */
	if (status != HAL_OK)
		/* Execute user timeout callback */
		I2Cx_Error(Addr);
}

/**
 * @brief  Reads a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @retval Data to be read
 */
uint8_t CODEC_Read(uint8_t Addr, uint8_t Reg) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t value = 0;

	status = HAL_I2C_Mem_Read(pi2c, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT,
			&value, 1, I2Cx_TIMEOUT_MAX);

	/* Check the communication status */
	if (status != HAL_OK)
		/* Execute user timeout callback */
		I2Cx_Error(Addr);

	return value;
}

/**
 * @brief  Writes/Read a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  Value: Data to be written
 * @retval None
 */
uint8_t CODEC_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value) {
	uint32_t result = 0;

	CODEC_Write(Addr, Reg, Value);

#ifdef VERIFY_WRITTENDATA
	/* Verify that the data has been correctly written */
	result = (CODEC_Read(Addr, Reg) == Value) ? 0 : 1;
#endif /* VERIFY_WRITTENDATA */

	return result;
}

/* Private functions implementations ---------------------------------------------*/

/**
 * @brief  Manages error callback by re-initializing I2C.
 * @param  Addr: I2C Address
 */
static void I2Cx_Error(uint8_t Addr) {
	/* De-initialize the I2C communication bus */
	HAL_I2C_MspDeInit(pi2c);

	/* Re-Initialize the I2C communication bus */
	HAL_I2C_MspInit(pi2c);
}

static uint8_t CODEC_Check(uint8_t Addr) {
	return (HAL_I2C_IsDeviceReady(pi2c, Addr, 10, 1000) == HAL_OK);
}

