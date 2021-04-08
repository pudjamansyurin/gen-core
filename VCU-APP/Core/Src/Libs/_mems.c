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
		.drag = {
				.init = 1,
				.tilt_cur = {0},
				.tilt_ref = {0},
		},
		.pi2c = &hi2c3,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t Capture(mems_raw_t *raw);
static void ConvertAccel(mems_tilt_t *tilt, mems_axis_t *axis);
#if MEMS_DEBUG
static void Debugger(move_t *move);
static void RawDebugger(mems_raw_t *raw);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t MEMS_Init(void) {
	uint8_t ok;
	uint32_t tick;

	lock();
	printf("MEMS:Init\n");
	tick = _GetTickMS();
	do {
		MX_I2C3_Init();
		GATE_MemsReset();

		ok = MPU6050_Init(MEMS.pi2c, &(MEMS.dev), MPU6050_Device_0,	MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s) == MPU6050_Result_Ok;
		if (!ok) _DelayMS(500);
	} while (!ok && _GetTickMS() - tick < MEMS_TIMEOUT);

	if (ok) MEMS.d.tick = _GetTickMS();
	unlock();

	printf("MEMS:%s\n", ok ? "OK" : "Error");
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
	mems_tilt_t *tilt = &(MEMS.drag.tilt_cur);

	lock();
	ConvertAccel(tilt, &(raw->accelerometer));
	MEMS.d.tot.accelerometer = sqrt(
			pow(raw->accelerometer.x, 2) +
			pow(raw->accelerometer.y, 2) +
			pow(raw->accelerometer.z, 2)
	);
	MEMS.d.tot.gyroscope = sqrt(
			pow(raw->gyroscope.x, 2) +
			pow(raw->gyroscope.y, 2) +
			pow(raw->gyroscope.z, 2)
	);
	MEMS.d.tot.tilt = sqrt(
			pow(tilt->roll, 2) +
			pow(tilt->pitch, 2)
	);

	MEMS.d.crash = MEMS.d.tot.accelerometer > CRASH_LIMIT;
	MEMS.d.fall = MEMS.d.tot.tilt > FALL_LIMIT;

#if MEMS_DEBUG
	Debugger(move);
#endif
	unlock();

	return (MEMS.d.crash || MEMS.d.fall);
}

uint8_t MEMS_ActivateDetector(void) {
	mems_tilt_t *cur = &(MEMS.drag.tilt_cur);
	mems_tilt_t *ref = &(MEMS.drag.tilt_ref);
	uint8_t init = MEMS.drag.init;

	lock();
	if (init) {
		MEMS.drag.init = 0;
		memcpy(ref, cur, sizeof(mems_tilt_t));
		VCU.SetEvent(EVG_BIKE_MOVED, 0);
	}
	unlock();

	return init == 0;
}

uint8_t MEMS_Dragged(void) {
	mems_tilt_t *cur = &(MEMS.drag.tilt_cur);
	mems_tilt_t *ref = &(MEMS.drag.tilt_ref);
	uint8_t euclidean, dragged;

	lock();
	euclidean = sqrt(
			pow(ref->roll - cur->roll, 2) +
			pow(ref->pitch - cur->pitch, 2)
	);

	dragged = euclidean > DRAGGED_LIMIT;
	if (dragged)
		printf("MEMS:Gyro dragged = %d\n", euclidean);
	unlock();

	return dragged;
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
		raw->gyroscope.y = dev->Gyroscope_Y * dev->Gyro_Mult; // reverse Yaw & Roll
		raw->gyroscope.z = dev->Gyroscope_Z * dev->Gyro_Mult; // reverse Yaw & Roll
		// temperature
		raw->temperature = dev->Temperature;
	}

	return ok;
}

static void ConvertAccel(mems_tilt_t *tilt, mems_axis_t *axis) {
	float pitch, roll;

	roll = sqrt(pow(axis->x, 2) + pow(axis->z, 2));
	pitch = sqrt(pow(axis->y, 2) + pow(axis->z, 2));

	tilt->roll = roll == 0 ? 0 : RAD2DEG(atan(axis->y / roll));
	tilt->pitch = pitch == 0 ? 0 : RAD2DEG(atan(axis->x / pitch));
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
