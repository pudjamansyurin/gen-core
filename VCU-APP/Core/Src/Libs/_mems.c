/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Libs/_mems.h"
#include "Nodes/VCU.h"
#include "i2c.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t MemsRecMutexHandle;
#endif

/* Public variables
 * ----------------------------------------------------------*/
mems_t MEMS = {
		.d = {0},
		.move = {
				.fall = {0},
				.crash = {0},
				.fallen = 0,
		},
		.drag = {0},
		.pi2c = &hi2c3,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t Capture(mems_raw_t *raw);
static void ConvertGyro(gyroscope_t *gyro, coordinate_t *coor);
static uint8_t Moved(gyroscope_t *ref, gyroscope_t *now);
#if MEMS_DEBUG
static void Debugger(move_t *move);
static void RawDebugger(mems_raw_t *raw);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t MEMS_Init(void) {
	uint8_t ok;

	lock();
	uint32_t tick = _GetTickMS();
	do {
		printf("MEMS:Init\n");

		MX_I2C3_Init();
		GATE_MemsReset();

		ok = MPU6050_Init(MEMS.pi2c, &(MEMS.dev), MPU6050_Device_0,	MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s) == MPU6050_Result_Ok;
		_DelayMS(500);
	} while (!ok && _GetTickMS() - tick < MEMS_TIMEOUT);
	unlock();

	return ok;
}

void MEMS_DeInit(void) {
	lock();
	MEMS_Flush();
	GATE_MemsShutdown();
	HAL_I2C_DeInit(MEMS.pi2c);
	unlock();
}


void MEMS_Refresh(void) {
	lock();
	MEMS.d.active = MEMS.d.tick && (_GetTickMS() - MEMS.d.tick) < MEMS_TIMEOUT;
	if (!MEMS.d.active) {
		MEMS_DeInit();
		_DelayMS(500);
		MEMS_Init();
	}
	unlock();
}

void MEMS_Flush(void) {
	memset(&(MEMS.d), 0, sizeof(mems_data_t));
	memset(&(MEMS.move), 0, sizeof(move_t));
	memset(&(MEMS.drag), 0, sizeof(drag_t));
}


uint8_t MEMS_Capture(void) {
	gyroscope_t *gyro = &(MEMS.d.gyro);
	mems_raw_t *raw = &(MEMS.d.raw);
	uint8_t ok;

	lock();
	ok = Capture(raw);

	if (ok) {
		MEMS.d.tick = _GetTickMS();
		ConvertGyro(gyro, &(raw->gyroscope));
#if MEMS_DEBUG
		RawDebugger(raw);
#endif
	}
	unlock();

	return ok;
}

uint8_t MEMS_Process(void) {
	gyroscope_t *gyro = &(MEMS.d.gyro);
	mems_raw_t *raw = &(MEMS.d.raw);
	move_t *move = &(MEMS.move);

	lock();
	// calculate accel
	move->crash.value =
			sqrt(
					pow(raw->accelerometer.x, 2) + pow(raw->accelerometer.y, 2) + pow(raw->accelerometer.z, 2)
			) /	GRAVITY_FORCE;
	move->crash.state = move->crash.value > ACCELEROMETER_LIMIT;

	// calculate gyro
	move->fall.value = sqrt(pow(gyro->roll, 2) + pow(gyro->pitch, 2) + pow(gyro->yaw, 2));
	move->fall.state = move->fall.value > GYROSCOPE_LIMIT;

	// fallen indicator
	move->fallen = move->crash.state || move->fall.state;

#if MEMS_DEBUG
	Debugger(move);
#endif
	unlock();

	return move->fallen;
}

void MEMS_ActivateDetector(void) {
	gyroscope_t *gyro = &(MEMS.d.gyro);
	gyroscope_t *ref = &(MEMS.drag.gyro);

	lock();
	if (MEMS.drag.init) {
		MEMS.drag.init = 0;
		memcpy(ref, gyro, sizeof(gyroscope_t));
		VCU.SetEvent(EVG_BIKE_MOVED, 0);
	}
	else if (Moved(ref, gyro))
		VCU.SetEvent(EVG_BIKE_MOVED, 1);
	unlock();
}

void MEMS_ResetDetector(void) {
	lock();
	MEMS.drag.init = 1;
	VCU.SetEvent(EVG_BIKE_MOVED, 0);
	unlock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(MemsRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
	osMutexRelease(MemsRecMutexHandle);
#endif
}

static uint8_t Capture(mems_raw_t *raw) {
	MPU6050 *dev = &(MEMS.dev);
	uint8_t ok;

	// read sensor
	ok = MPU6050_ReadAll(MEMS.pi2c, &(MEMS.dev)) == MPU6050_Result_Ok;

	if (ok) {
		// convert the RAW values into dps (deg/s)
		raw->accelerometer.x = dev->Gyroscope_X / dev->Gyro_Mult;
		raw->accelerometer.y = dev->Gyroscope_Y / dev->Gyro_Mult;
		raw->accelerometer.z = dev->Gyroscope_Z / dev->Gyro_Mult;
		// convert the RAW values into acceleration in 'g'
		raw->gyroscope.x = dev->Accelerometer_X / dev->Acce_Mult;
		raw->gyroscope.y = dev->Accelerometer_Y / dev->Acce_Mult;
		raw->gyroscope.z = dev->Accelerometer_Z / dev->Acce_Mult;
		// temperature
		raw->temperature = dev->Temperature;
	}

	return ok;
}

static void ConvertGyro(gyroscope_t *gyro, coordinate_t *coor) {
	float yaw, pitch, roll;

	// Calculating Roll and Pitch from the accelerometer data
	yaw = sqrt(pow(coor->x, 2) + pow(coor->y, 2));
	roll = sqrt(pow(coor->x, 2) + pow(coor->z, 2));
	pitch = sqrt(pow(coor->y, 2) + pow(coor->z, 2));

	gyro->yaw = yaw == 0 ? 0 : RAD2DEG(atan(coor->z / yaw));
	gyro->roll = roll == 0 ? 0 : RAD2DEG(atan(coor->y / roll));
	gyro->pitch = pitch == 0 ? 0 : RAD2DEG(atan(coor->x / pitch));

	// normalize yaw axes
	gyro->yaw -= 90;
}

static uint8_t Moved(gyroscope_t *ref, gyroscope_t *now) {
	uint8_t euclidean;

	euclidean = sqrt(pow(ref->roll - now->roll, 2) +
			pow(ref->pitch - now->pitch, 2) +
			pow(ref->yaw - now->yaw, 2));

	if (euclidean > MOVED_LIMIT)
		printf("IMU:Gyro moved = %d\n", euclidean);
	return euclidean > MOVED_LIMIT;
}

#if MEMS_DEBUG
static void Debugger(move_t *move) {
	printf("IMU:Accel[%lu %%] = %lu / %lu\n",
			move->crash.value * 100 / ACCELEROMETER_LIMIT,
			move->crash.value, ACCELEROMETER_LIMIT);
	printf("IMU:Gyros[%lu %%] = %lu / %u\n",
			move->fall.value * 100 / GYROSCOPE_LIMIT, move->fall.value,
			GYROSCOPE_LIMIT);
}

static void RawDebugger(mems_raw_t *raw) {
	printf("Acce:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
			"Gyro:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
			"Temp: %d\n\n",
			raw->accelerometer.x,raw->accelerometer.y,raw->accelerometer.z,
			raw->gyroscope.x,raw->gyroscope.y,raw->gyroscope.z,
			(int8_t)raw->temperature);
}
#endif
