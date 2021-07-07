/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Pudja Mansyurin
 */
/* Includes
 * --------------------------------------------*/
#include "Libs/_mems.h"

#include <math.h>

#include "Nodes/VCU.h"
#include "i2c.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t MemsRecMutexHandle;
#endif

/* Public variables
 * --------------------------------------------*/
mems_t MEMS = {
    .d = {0},
    .det = {.active = 0,
            .offset = 0,
            .tilt =
                {
                    .cur = {0},
                    .ref = {0},
                }},
    .pi2c = &hi2c3,
};

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t Capture(mems_raw_t *raw);
static void ConvertAccel(mems_tilt_t *tilt, mems_axis_t *axis);
static uint8_t OnlyGotTemp(void);
#if MEMS_DEBUG
static void Debugger(mems_total_t *tot);
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

    ok = MPU_Init(MEMS.pi2c, &(MEMS.dev), MPU_Device_0, MPU_Accel_16G,
                  MPU_Gyro_2000s) == MPUR_Ok;
    if (!ok) _DelayMS(500);
  } while (!ok && _TickIn(tick, MEMS_TIMEOUT_MS));

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
  MEMS.d.active = _TickIn(MEMS.d.tick, MEMS_TIMEOUT_MS);

  // handle bug, when only got temperature
  if (MEMS.d.active) MEMS.d.active = !OnlyGotTemp();

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
  memset(&(MEMS.sample), 0, sizeof(mems_sample_t));
  memset(&(MEMS.det), 0, sizeof(mems_detector_t));
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
  mems_tilt_t *tilt = &(MEMS.det.tilt.cur);
  mems_sample_t *s = &(MEMS.sample);

  lock();
  ConvertAccel(tilt, &(raw->accel));

  MEMS.d.tot.accel = _SamplingFloat(
      &s->handle[MEMS_SAMPLE_ACCEL], s->buffer[MEMS_SAMPLE_ACCEL],
      MEMS_SAMPLE_SZ,
      sqrt(pow(raw->accel.x, 2) + pow(raw->accel.y, 2) + pow(raw->accel.z, 2)));
  MEMS.d.tot.gyro = _SamplingFloat(
      &s->handle[MEMS_SAMPLE_GYRO], s->buffer[MEMS_SAMPLE_GYRO], MEMS_SAMPLE_SZ,
      sqrt(pow(raw->gyro.x, 2) + pow(raw->gyro.y, 2) + pow(raw->gyro.z, 2)));
  MEMS.d.tot.tilt = _SamplingFloat(
      &s->handle[MEMS_SAMPLE_TILT], s->buffer[MEMS_SAMPLE_TILT], MEMS_SAMPLE_SZ,
      sqrt(pow(tilt->roll, 2) + pow(tilt->pitch, 2)));

  MEMS.d.crash = MEMS.d.tot.accel > CRASH_LIMIT;
  MEMS.d.fall = MEMS.d.tot.tilt > FALL_LIMIT;

#if MEMS_DEBUG
  Debugger(&(MEMS.d.tot));
#endif
  unlock();

  return (MEMS.d.crash || MEMS.d.fall);
}

void MEMS_GetRefDetector(void) {
  mems_tilt_t *cur = &(MEMS.det.tilt.cur);
  mems_tilt_t *ref = &(MEMS.det.tilt.ref);

  lock();
  EVT_Clr(EVG_BIKE_MOVED);
  memcpy(ref, cur, sizeof(mems_tilt_t));
  unlock();
}

uint8_t MEMS_Dragged(void) {
  mems_tilt_t *cur = &(MEMS.det.tilt.cur);
  mems_tilt_t *ref = &(MEMS.det.tilt.ref);
  uint8_t euclidean, dragged;

  lock();
  euclidean =
      sqrt(pow(ref->roll - cur->roll, 2) + pow(ref->pitch - cur->pitch, 2));
  MEMS.det.offset = euclidean;
  unlock();

  dragged = euclidean > DRAGGED_LIMIT;
#if MEMS_DEBUG
  if (dragged) printf("MEMS:Gyro dragged = %d\n", euclidean);
#endif
  return dragged;
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(MemsRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(MemsRecMutexHandle);
#endif
}

static uint8_t Capture(mems_raw_t *raw) {
  MPU6050 *dev = &(MEMS.dev);
  uint8_t ok;

  // read sensor
  ok = MPU_ReadAll(dev) == MPUR_Ok;

  if (ok) {
    // convert the RAW values into acceleration in 'g'
    raw->accel.x = dev->Accel_X * dev->Acce_Mult;
    raw->accel.y = dev->Accel_Y * dev->Acce_Mult;
    raw->accel.z = dev->Accel_Z * dev->Acce_Mult;
    // convert the RAW values into dps (deg/s)
    raw->gyro.x = dev->Gyro_X * dev->Gyro_Mult;
    raw->gyro.y = dev->Gyro_Y * dev->Gyro_Mult;  // reverse Yaw & Roll
    raw->gyro.z = dev->Gyro_Z * dev->Gyro_Mult;  // reverse Yaw & Roll
    // temperature
    raw->temp = dev->Temp;
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

static uint8_t OnlyGotTemp(void) {
  mems_raw_t *raw = &(MEMS.d.raw);
  mems_axis_t axis = {0};
  uint8_t empty;

  empty = memcmp(&(raw->accel), &axis, sizeof(mems_axis_t)) == 0;
  empty &= memcmp(&(raw->gyro), &axis, sizeof(mems_axis_t)) == 0;

  return empty;
}

#if MEMS_DEBUG
static void Debugger(mems_total_t *tot) {
  printf("MEMS:Accel[%lu %%] = %lu / %u\n",
         (uint32_t)(tot->accel * 100 / CRASH_LIMIT), (uint32_t)tot->accel,
         (uint8_t)CRASH_LIMIT);
  printf("MEMS:Gyros[%lu %%] = %lu / %u\n",
         (uint32_t)(tot->tilt * 100 / FALL_LIMIT), (uint32_t)tot->tilt,
         (uint8_t)FALL_LIMIT);
}

static void RawDebugger(mems_raw_t *raw) {
  printf(
      "Acce:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
      "Gyro:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
      "Temp: %d\n\n",
      (int32_t)raw->accel.x, (int32_t)raw->accel.y, (int32_t)raw->accel.z,
      (int32_t)raw->gyro.x, (int32_t)raw->gyro.y, (int32_t)raw->gyro.z,
      (int8_t)raw->temp);
}
#endif
