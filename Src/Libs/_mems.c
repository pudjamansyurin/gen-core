/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
#include <_mems.h>

void MEMS_Init(I2C_HandleTypeDef *i2c, SD_MPU6050 *mpu) {
	SD_MPU6050_Result result;

	do {
		SWV_SendStrLn("MEMS_Init");
		// turn off module
		HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, GPIO_PIN_RESET);
		osDelay(1000);
		// turn on module
		HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, GPIO_PIN_SET);
		osDelay(1000);
		// module initialization
		result = SD_MPU6050_Init(i2c, mpu, SD_MPU6050_Device_0, SD_MPU6050_Accelerometer_16G, SD_MPU6050_Gyroscope_250s);
	} while (result != SD_MPU6050_Result_Ok);
}

mems_t MEMS_Average(I2C_HandleTypeDef *i2c, SD_MPU6050 *mpu, mems_t *calibrator, uint16_t sample) {
	uint16_t i;
	mems_t mems;
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
		SD_MPU6050_ReadAll(i2c, mpu);
		// sum all value
		mems.accelerometer.x += mpu->Gyroscope_X;
		mems.accelerometer.y += mpu->Gyroscope_Y;
		mems.accelerometer.z += mpu->Gyroscope_Z;
		mems.gyroscope.x += mpu->Accelerometer_X;
		mems.gyroscope.y += mpu->Accelerometer_Y;
		mems.gyroscope.z += mpu->Accelerometer_Z;
	}
	// calculate the average
	mems.accelerometer.x = mems.accelerometer.x / sample;
	mems.accelerometer.y = mems.accelerometer.y / sample;
	mems.accelerometer.z = mems.accelerometer.z / sample;
	mems.gyroscope.x = mems.gyroscope.x / sample;
	mems.gyroscope.y = mems.gyroscope.y / sample;
	mems.gyroscope.z = mems.gyroscope.z / sample;
	// set for calibration
	if (calibrator != NULL) {
		mems.accelerometer.x -= calibrator->accelerometer.x;
		mems.accelerometer.y -= calibrator->accelerometer.y;
		mems.accelerometer.z -= calibrator->accelerometer.z;
		mems.gyroscope.x -= calibrator->gyroscope.x;
		mems.gyroscope.y -= calibrator->gyroscope.y;
		mems.gyroscope.z -= calibrator->gyroscope.z;
	}

	return mems;
}

mems_decision_t MEMS_Decision(I2C_HandleTypeDef *i2c, SD_MPU6050 *mpu, mems_t *calibrator, uint16_t sample) {
	int32_t g_force, accel_limit = 46000, gyro_limit = 10000;
	mems_decision_t mems_decision;
	mems_t mems;
	//	char str[200];

	// get mems data
	mems = MEMS_Average(i2c, mpu, calibrator, sample);
	// calculate g-force
	g_force = sqrt(pow(mems.accelerometer.x, 2) + pow(mems.accelerometer.y, 2) + pow(mems.accelerometer.z, 2));
	mems_decision.crash = (g_force > accel_limit);
	// calculate movement change
	mems_decision.fall = (abs(mems.gyroscope.z) > gyro_limit);

	// for debugging
	//	sprintf(str,
	//			"Accelerometer\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
	//			"Gyroscope\n- X:%ld\n- Y:%ld\n- Z:%ld\n\n",
	//			mems.accelerometer.x, mems.accelerometer.y, mems.accelerometer.z,
	//			mems.gyroscope.x, mems.gyroscope.y, mems.gyroscope.z
	//	);
	//	SWV_SendStr(str);

	return mems_decision;
}
