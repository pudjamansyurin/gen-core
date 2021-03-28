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
		.drag = {0},
		.pi2c = &hi2c3,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t Capture(mems_raw_t *raw);
static void ConvertGyro(gyroscope_t *gyro, mems_axis_t *axis);
static uint8_t Dragged(gyroscope_t *ref, gyroscope_t *cur);
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
	lock();
	memset(&(MEMS.d), 0, sizeof(mems_data_t));
	memset(&(MEMS.drag), 0, sizeof(drag_t));
	unlock();
}


uint8_t MEMS_Capture(void) {
	mems_raw_t *raw = &(MEMS.d.raw);
	uint8_t ok;

	lock();
	ok = Capture(raw);

	if (ok) {
		MEMS.d.tick = _GetTickMS();
#if MEMS_DEBUG
		RawDebugger(raw);
#endif
	}
	unlock();

	return ok;
}

uint8_t MEMS_Process(void) {
	mems_raw_t *raw = &(MEMS.d.raw);
	gyroscope_t *gyro = &(MEMS.drag.gyro_cur);

	lock();
	ConvertGyro(gyro, &(raw->gyroscope));
	MEMS.d.tot.accelerometer = sqrt(
			pow(raw->accelerometer.x, 2) +
			pow(raw->accelerometer.y, 2) +
			pow(raw->accelerometer.z, 2)
	);
	MEMS.d.tot.gyroscope = sqrt(
			pow(gyro->roll, 2) +
			pow(gyro->pitch, 2) +
			pow(gyro->yaw, 2)
	);

	MEMS.d.crash = MEMS.d.tot.accelerometer > ACCELEROMETER_LIMIT;
	MEMS.d.fall = MEMS.d.tot.gyroscope > GYROSCOPE_LIMIT;

#if MEMS_DEBUG
	Debugger(move);
#endif
	unlock();

	return (MEMS.d.crash || MEMS.d.fall);
}

void MEMS_ActivateDetector(void) {
	gyroscope_t *gyro_cur = &(MEMS.drag.gyro_cur);
	gyroscope_t *gyro_ref = &(MEMS.drag.gyro_ref);

	lock();
	if (MEMS.drag.init) {
		MEMS.drag.init = 0;
		memcpy(gyro_ref, gyro_cur, sizeof(gyroscope_t));
		VCU.SetEvent(EVG_BIKE_MOVED, 0);
	}
	else if (Dragged(gyro_ref, gyro_cur))
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
		// convert the RAW values into acceleration in 'g'
		raw->accelerometer.x = dev->Accelerometer_X * dev->Acce_Mult;
		raw->accelerometer.y = dev->Accelerometer_Y * dev->Acce_Mult;
		raw->accelerometer.z = dev->Accelerometer_Z * dev->Acce_Mult;
		// convert the RAW values into dps (deg/s)
		raw->gyroscope.x = dev->Gyroscope_X * dev->Gyro_Mult;
		raw->gyroscope.z = dev->Gyroscope_Y * dev->Gyro_Mult; // reverse Yaw & Roll
		raw->gyroscope.y = dev->Gyroscope_Z * dev->Gyro_Mult; // reverse Yaw & Roll
		// temperature
		raw->temperature = dev->Temperature;
	}

	return ok;
}

static void ConvertGyro(gyroscope_t *gyro, mems_axis_t *axis) {
	float yaw, pitch, roll;

	yaw = sqrt(pow(axis->x, 2) + pow(axis->y, 2));
	roll = sqrt(pow(axis->x, 2) + pow(axis->z, 2));
	pitch = sqrt(pow(axis->y, 2) + pow(axis->z, 2));

	gyro->yaw = yaw == 0 ? 0 : RAD2DEG(atan(axis->z / yaw));
	gyro->roll = roll == 0 ? 0 : RAD2DEG(atan(axis->y / roll));
	gyro->pitch = pitch == 0 ? 0 : RAD2DEG(atan(axis->x / pitch));

	// normalize yaw axis
	gyro->yaw -= 90;
}

static uint8_t Dragged(gyroscope_t *ref, gyroscope_t *cur) {
	uint8_t euclidean;

	euclidean = sqrt(pow(ref->roll - cur->roll, 2) +
			pow(ref->pitch - cur->pitch, 2) +
			pow(ref->yaw - cur->yaw, 2));

	if (euclidean > DRAGGED_LIMIT)
		printf("MEMS:Gyro dragged = %d\n", euclidean);
	return euclidean > DRAGGED_LIMIT;
}

#if MEMS_DEBUG
static void Debugger(move_t *move) {
	printf("MEMS:Accel[%lu %%] = %lu / %lu\n",
			move->crash.value * 100 / ACCELEROMETER_LIMIT,
			move->crash.value, ACCELEROMETER_LIMIT);
	printf("MEMS:Gyros[%lu %%] = %lu / %u\n",
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
