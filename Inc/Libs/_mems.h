/*
 * mems.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */

#ifndef MEMS_H_
#define MEMS_H_

#include <stdio.h>				// for: sprintf()
#include <stdlib.h>				// for: abs()
#include <math.h>					// for: pow(), sqrt()
#include "main.h"
#include "cmsis_os.h"
#include "_mpu6050.h"
#include "_swv.h"

/* Public typedef -----------------------------------------------------------*/
typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} coordinate_t;

typedef struct {
	coordinate_t accelerometer;
	coordinate_t gyroscope;
} mems_t;

typedef struct {
	uint8_t fall;
	uint8_t crash;
} mems_decision_t;

/* Public functions ---------------------------------------------------------*/
void MEMS_Init(I2C_HandleTypeDef *i2c, SD_MPU6050 *mpu);
mems_t MEMS_Average(I2C_HandleTypeDef *i2c, SD_MPU6050 *mpu, mems_t *calibrator, uint16_t sample);
mems_decision_t MEMS_Decision(I2C_HandleTypeDef *i2c, SD_MPU6050 *mpu, mems_t *calibrator, uint16_t sample);

#endif /* MEMS_H_ */
