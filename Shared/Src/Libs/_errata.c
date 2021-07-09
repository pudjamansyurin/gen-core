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

/* Public functions implementation
 * --------------------------------------------*/
void ERRATA_I2C_ClearBusyFlag(void) {
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
