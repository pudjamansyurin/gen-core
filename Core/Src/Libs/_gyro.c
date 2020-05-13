/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Libs/_gyro.h"
#include "Drivers/_mpu6050.h"

/* External variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c3;

/* Private variables ----------------------------------------------------------*/
static MPU6050 mpu;
static mems_t mems, calibrator;
static uint8_t calibrated;
static mems_decision_t decider;

/* Public functions implementation --------------------------------------------*/
void GYRO_Init(void) {
	MPU6050_Result result;
	calibrated = 0;

	do {
		LOG_StrLn("Gyro:Init");

		// MOSFET Control
		HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 0);
		osDelay(500);
		HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 1);
		osDelay(1000);

		// module initialization
		result = MPU6050_Init(&hi2c3, &mpu, MPU6050_Device_0, MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s);

	} while (result != MPU6050_Result_Ok);

	// Set calibrator
	calibrator = GYRO_Average(500);
	calibrated = 1;
	LOG_StrLn("Gyro:Calibrated");
}

mems_t GYRO_Average(uint16_t sample) {
	uint16_t i;

	// reset value
	mems.accelerometer.x = 0;
	mems.accelerometer.y = 0;
	mems.accelerometer.z = 0;
	mems.gyroscope.x = 0;
	mems.gyroscope.y = 0;
	mems.gyroscope.z = 0;

	// sampling
	for (i = 0; i < sample; i++) {
		// read sensor
		MPU6050_ReadAll(&hi2c3, &mpu);
		// sum all value
		mems.accelerometer.x += mpu.Gyroscope_X;
		mems.accelerometer.y += mpu.Gyroscope_Y;
		mems.accelerometer.z += mpu.Gyroscope_Z;
		mems.gyroscope.x += mpu.Accelerometer_X;
		mems.gyroscope.y += mpu.Accelerometer_Y;
		mems.gyroscope.z += mpu.Accelerometer_Z;
	}

	// calculate the average
	mems.accelerometer.x = mems.accelerometer.x / sample;
	mems.accelerometer.y = mems.accelerometer.y / sample;
	mems.accelerometer.z = mems.accelerometer.z / sample;
	mems.gyroscope.x = mems.gyroscope.x / sample;
	mems.gyroscope.y = mems.gyroscope.y / sample;
	mems.gyroscope.z = mems.gyroscope.z / sample;

	// set for calibration
	if (calibrated) {
		mems.accelerometer.x -= calibrator.accelerometer.x;
		mems.accelerometer.y -= calibrator.accelerometer.y;
		mems.accelerometer.z -= calibrator.accelerometer.z;
		mems.gyroscope.x -= calibrator.gyroscope.x;
		mems.gyroscope.y -= calibrator.gyroscope.y;
		mems.gyroscope.z -= calibrator.gyroscope.z;
	}

	return mems;
}

mems_decision_t GYRO_Decision(uint16_t sample) {
	// get mems data
	mems = GYRO_Average(sample);

	// calculate g-force
	decider.crash.value = sqrt(pow(mems.accelerometer.x, 2) +
			pow(mems.accelerometer.y, 2) +
			pow(mems.accelerometer.z, 2));
	decider.crash.state = (decider.crash.value > ACCELEROMETER_LIMIT);

	// calculate movement change
	decider.fall.value = (abs(mems.gyroscope.z));
	decider.fall.state = decider.fall.value > GYROSCOPE_LIMIT;

	// debugger
//	Gyro_RawDebugger();

	return decider;
}

void Gyro_Debugger(mems_decision_t *decider) {
	// calculated data
	LOG_Str("IMU:Accel[");
	LOG_Int(decider->crash.value * 100 / ACCELEROMETER_LIMIT);
	LOG_Str(" %] = ");
	LOG_Int(decider->crash.value);
	LOG_Str(" / ");
	LOG_Int(ACCELEROMETER_LIMIT);
	LOG_Enter();
	LOG_Str("IMU:Gyros[");
	LOG_Int(decider->fall.value * 100 / GYROSCOPE_LIMIT);
	LOG_Str(" %] = ");
	LOG_Int(decider->fall.value);
	LOG_Str(" / ");
	LOG_Int(GYROSCOPE_LIMIT);
	LOG_Enter();
}

void Gyro_RawDebugger(void) {
	// raw data
	char str[100];
	sprintf(str,
			"Accelerometer\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
					"Gyroscope\n- X:%ld\n- Y:%ld\n- Z:%ld\n\n",
			mems.accelerometer.x, mems.accelerometer.y, mems.accelerometer.z,
			mems.gyroscope.x, mems.gyroscope.y, mems.gyroscope.z
			);
	LOG_Str(str);
}
