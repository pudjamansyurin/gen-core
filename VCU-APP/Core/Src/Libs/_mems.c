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

/* Private constants
 * --------------------------------------------*/
#define MEMS_SAMPLE_SZ ((uint8_t)10)
#define MEMS_TIMEOUT_MS ((uint16_t)5000)

#define DRAGGED_LIMIT ((uint8_t)5)
#define FALL_LIMIT ((uint8_t)60)
#define CRASH_LIMIT ((uint8_t)20)

/* Private enums
 * --------------------------------------------*/
typedef enum {
  MSAMPLE_ACCEL = 0,
  MSAMPLE_GYRO,
  MSAMPLE_TILT,
  MSAMPLE_MAX,
} MEMS_SAMPLE;

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint8_t active;
  uint8_t offset;
  mems_tilt_t tilt[MTILT_MAX];
} mems_motion_t;

typedef struct {
  uint8_t active;
  uint32_t tick;
  uint8_t effect[MEFFECT_MAX];
  mems_raw_t raw;
  mems_total_t total;
} mems_data_t;

typedef struct {
  sample_float_t handle[MSAMPLE_MAX];
  float buffer[MSAMPLE_MAX][MEMS_SAMPLE_SZ];
} mems_sample_t;

typedef struct {
  mems_data_t d;
  mems_sample_t sample;
  mems_motion_t motion;

  I2C_HandleTypeDef *pi2c;
} mems_t;

/* Private variables
 * --------------------------------------------*/
static mems_t MEMS = {
    .d = {0},
    .motion = {0},
    .pi2c = &hi2c3,
};

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t Capture(void);
static void ConvertAccel(void);
static uint8_t OnlyGotTemp(void);
#if MEMS_DEBUG
static void Debugger(void);
static void RawDebugger(void);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t MEMS_Init(void) {
  uint8_t ok;
  uint32_t tick;

  lock();
  printf("MEMS:Init\n");
  tick = tickMs();
  do {
    MX_I2C3_Init();
    GATE_MemsReset();

    ok = MPU_Init(MEMS.pi2c, MPU_Device_0, MPU_Accel_16G, MPU_Gyro_2000s) ==
         MPUR_Ok;
    if (!ok) delayMs(500);
  } while (!ok && tickIn(tick, MEMS_TIMEOUT_MS));

  if (ok) MEMS.d.tick = tickMs();
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
  MEMS.d.active = tickIn(MEMS.d.tick, MEMS_TIMEOUT_MS);

  // handle bug, when only got temperature
  if (MEMS.d.active) MEMS.d.active = !OnlyGotTemp();

  if (!MEMS.d.active) {
    MEMS_DeInit();
    delayMs(500);
    MEMS_Init();
  }
  unlock();
}

void MEMS_Flush(void) {
  lock();
  memset(&(MEMS.d), 0, sizeof(mems_data_t));
  memset(&(MEMS.sample), 0, sizeof(mems_sample_t));
  memset(&(MEMS.motion), 0, sizeof(mems_motion_t));
  unlock();
}

uint8_t MEMS_Capture(void) {
  uint8_t ok;

  lock();
  ok = Capture();

  if (ok) {
    MEMS.d.tick = tickMs();
#if MEMS_DEBUG
    RawDebugger();
#endif
  }
  unlock();

  return ok;
}

uint8_t MEMS_Process(void) {
  mems_raw_t *raw = &(MEMS.d.raw);
  mems_tilt_t *t = &(MEMS.motion.tilt[MTILT_NOW]);
  mems_sample_t *s = &(MEMS.sample);
  uint8_t *effect = MEMS.d.effect;

  lock();
  ConvertAccel();

  MEMS.d.total.accel = samplingFloat(
      &s->handle[MSAMPLE_ACCEL], s->buffer[MSAMPLE_ACCEL], MEMS_SAMPLE_SZ,
      sqrt(pow(raw->accel.x, 2) + pow(raw->accel.y, 2) + pow(raw->accel.z, 2)));
  MEMS.d.total.gyro = samplingFloat(
      &s->handle[MSAMPLE_GYRO], s->buffer[MSAMPLE_GYRO], MEMS_SAMPLE_SZ,
      sqrt(pow(raw->gyro.x, 2) + pow(raw->gyro.y, 2) + pow(raw->gyro.z, 2)));
  MEMS.d.total.tilt =
      samplingFloat(&s->handle[MSAMPLE_TILT], s->buffer[MSAMPLE_TILT],
                    MEMS_SAMPLE_SZ, sqrt(pow(t->roll, 2) + pow(t->pitch, 2)));

  effect[MEFFECT_CRASH] = MEMS.d.total.accel > CRASH_LIMIT;
  effect[MEFFECT_FALL] = MEMS.d.total.tilt > FALL_LIMIT;

#if MEMS_DEBUG
  Debugger();
#endif
  unlock();

  return (effect[MEFFECT_CRASH] || effect[MEFFECT_FALL]);
}

void MEMS_CaptureMotion(void) {
  mems_tilt_t *t = MEMS.motion.tilt;

  lock();
  EVT_Clr(EVG_BIKE_MOVED);
  memcpy(&t[MTILT_REFF], &t[MTILT_NOW], sizeof(mems_tilt_t));
  unlock();
}

uint8_t MEMS_Dragged(void) {
  uint8_t euclidean, dragged;
  mems_tilt_t *t = MEMS.motion.tilt;

  lock();
  euclidean = sqrt(pow(t[MTILT_REFF].roll - t[MTILT_NOW].roll, 2) +
                   pow(t[MTILT_REFF].pitch - t[MTILT_NOW].pitch, 2));
  MEMS.motion.offset = euclidean;
  unlock();

  dragged = euclidean > DRAGGED_LIMIT;
#if MEMS_DEBUG
  if (dragged) printf("MEMS:Gyro dragged = %d\n", euclidean);
#endif
  return dragged;
}

void MEMS_ToggleMotion(void) { MEMS.motion.active = !MEMS.motion.active; }

uint8_t MEMS_IO_Active(void) { return MEMS.d.active; }

uint8_t MEMS_IO_MotionActive(void) { return MEMS.motion.active; }

uint8_t MEMS_IO_MotionOffset(void) { return MEMS.motion.offset; }

const mems_raw_t *MEMS_IO_Raw(void) { return &(MEMS.d.raw); }

const mems_total_t *MEMS_IO_Total(void) { return &(MEMS.d.total); }

const mems_tilt_t *MEMS_IO_Tilt(MEMS_TILT key) {
  return &(MEMS.motion.tilt[key]);
}

uint8_t MEMS_IO_Effect(MEMS_EFFECT key) { return MEMS.d.effect[key] & 0x01; }

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

static uint8_t Capture(void) {
  mems_raw_t *raw = &(MEMS.d.raw);
  MPU_Dev dev;
  uint8_t ok;

  // read sensor
  ok = MPU_ReadAll(&dev) == MPUR_Ok;

  if (ok) {
    // convert the RAW values into acceleration in 'g'
    raw->accel.x = dev.Accel_X * dev.Acce_Mult;
    raw->accel.y = dev.Accel_Y * dev.Acce_Mult;
    raw->accel.z = dev.Accel_Z * dev.Acce_Mult;
    // convert the RAW values into dps (deg/s)
    raw->gyro.x = dev.Gyro_X * dev.Gyro_Mult;
    raw->gyro.y = dev.Gyro_Y * dev.Gyro_Mult;  // reverse Yaw & Roll
    raw->gyro.z = dev.Gyro_Z * dev.Gyro_Mult;  // reverse Yaw & Roll
    // temperature
    raw->temp = dev.Temp;
  }

  return ok;
}

static void ConvertAccel(void) {
  mems_axis_t *axis = &(MEMS.d.raw.accel);
  mems_tilt_t *tilt = &(MEMS.motion.tilt[MTILT_NOW]);

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
static void Debugger(void) {
  mems_total_t *total = &(MEMS.d.total);

  printf("MEMS:Accel[%lu %%] = %lu / %u\n",
         (uint32_t)(total->accel * 100 / CRASH_LIMIT), (uint32_t)total->accel,
         (uint8_t)CRASH_LIMIT);
  printf("MEMS:Gyros[%lu %%] = %lu / %u\n",
         (uint32_t)(total->tilt * 100 / FALL_LIMIT), (uint32_t)total->tilt,
         (uint8_t)FALL_LIMIT);
}

static void RawDebugger(void) {
  mems_raw_t *raw = &(MEMS.d.raw);

  printf(
      "Acce:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
      "Gyro:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
      "Temp: %d\n\n",
      (int32_t)raw->accel.x, (int32_t)raw->accel.y, (int32_t)raw->accel.z,
      (int32_t)raw->gyro.x, (int32_t)raw->gyro.y, (int32_t)raw->gyro.z,
      (int8_t)raw->temp);
}
#endif
