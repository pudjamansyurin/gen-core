/*
 * gyro.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Libs/_gyro.h"
#include "Nodes/VCU.h"
#include "i2c.h"


/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t GyroMutexHandle;
#endif

/* Private variables
 * ----------------------------------------------------------*/
static gyro_t gyro = {
    .detector_init = 1,
    .pi2c = &hi2c3,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static void Capture(mems_t *mems);
static void Convert(motion_t *motion, coordinate_t *gyro);
static uint8_t GYRO_Moved(motion_t *refference, motion_t *current);
static void Debugger(movement_t *movement);
static void RawDebugger(mems_t *mems);

/* Public functions implementation
 * --------------------------------------------*/
void GYRO_Init(void) {
  MPU6050_Result result;

  lock();
  do {
    printf("Gyro:Init\n");

    MX_I2C3_Init();
    GATE_GyroReset();

    // module initiate
    result = MPU6050_Init(gyro.pi2c, &(gyro.mpu), MPU6050_Device_0,
                          MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s);

  } while (result != MPU6050_Result_Ok);
  unlock();
}

void GYRO_DeInit(void) {
  GATE_GyroShutdown();
  HAL_I2C_DeInit(gyro.pi2c);
}

void GYRO_Decision(movement_t *movement) {
  motion_t *motion = &(VCU.d.motion);
  mems_t mems;

  lock();
  // get mems data
  Capture(&mems);
  Convert(motion, &(mems.gyroscope));

  // calculate accel
  movement->crash.value =
      sqrt(pow(mems.accelerometer.x, 2) + pow(mems.accelerometer.y, 2) +
           pow(mems.accelerometer.z, 2)) /
      GRAVITY_FORCE;
  movement->crash.state = movement->crash.value > ACCELEROMETER_LIMIT;

  // calculate gyro
  movement->fall.value =
      sqrt(pow(motion->roll, 2) + pow(motion->pitch, 2) + pow(motion->yaw, 2));
  movement->fall.state = movement->fall.value > GYROSCOPE_LIMIT;

  // fallen indicator
  movement->fallen = movement->crash.state || movement->fall.state;

  //  RawDebugger(&mems);
  //  Debugger(movement);
  unlock();
}

void GYRO_MonitorMovement(void) {
  motion_t *motion = &(VCU.d.motion);
  static motion_t refference;

  lock();
  if (gyro.detector_init) {
    gyro.detector_init = 0;
    memcpy(&refference, motion, sizeof(motion_t));
    VCU.SetEvent(EVG_BIKE_MOVED, 0);
  }

  if (GYRO_Moved(&refference, motion))
    VCU.SetEvent(EVG_BIKE_MOVED, 1);
  unlock();
}

void GYRO_ResetDetector(void) {
  lock();
  gyro.detector_init = 1;
  VCU.SetEvent(EVG_BIKE_MOVED, 0);
  unlock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
  osMutexAcquire(GyroMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
  osMutexRelease(GyroMutexHandle);
#endif
}

static void Capture(mems_t *mems) {
  MPU6050_Result status;

  // read sensor
  do {
    status = MPU6050_ReadAll(gyro.pi2c, &(gyro.mpu));

    if (status != MPU6050_Result_Ok)
      GYRO_Init();
  } while (status != MPU6050_Result_Ok);

  // convert the RAW values into dps (deg/s)
  mems->accelerometer.x = gyro.mpu.Gyroscope_X / gyro.mpu.Gyro_Mult;
  mems->accelerometer.y = gyro.mpu.Gyroscope_Y / gyro.mpu.Gyro_Mult;
  mems->accelerometer.z = gyro.mpu.Gyroscope_Z / gyro.mpu.Gyro_Mult;
  // convert the RAW values into acceleration in 'g'
  mems->gyroscope.x = gyro.mpu.Accelerometer_X / gyro.mpu.Acce_Mult;
  mems->gyroscope.y = gyro.mpu.Accelerometer_Y / gyro.mpu.Acce_Mult;
  mems->gyroscope.z = gyro.mpu.Accelerometer_Z / gyro.mpu.Acce_Mult;
  // temperature
  mems->temperature = gyro.mpu.Temperature;
}

static void Convert(motion_t *motion, coordinate_t *gyro) {
  motion_float_t mot;

  // Calculating Roll and Pitch from the accelerometer data
  mot.yaw = sqrt(pow(gyro->x, 2) + pow(gyro->y, 2));
  mot.roll = sqrt(pow(gyro->x, 2) + pow(gyro->z, 2));
  mot.pitch = sqrt(pow(gyro->y, 2) + pow(gyro->z, 2));

  motion->yaw = mot.yaw == 0 ? 0 : RAD2DEG(atan(gyro->z / mot.yaw));
  motion->roll = mot.roll == 0 ? 0 : RAD2DEG(atan(gyro->y / mot.roll));
  motion->pitch = mot.pitch == 0 ? 0 : RAD2DEG(atan(gyro->x / mot.pitch));

  // normalize yaw axes
  motion->yaw -= 90;
}

static uint8_t GYRO_Moved(motion_t *refference, motion_t *current) {
  uint8_t euclidean;

  euclidean = sqrt(pow(refference->roll - current->roll, 2) +
                   pow(refference->pitch - current->pitch, 2) +
                   pow(refference->yaw - current->yaw, 2));

  if (euclidean > MOVED_LIMIT)
    printf("IMU:Gyro moved = %d\n", euclidean);
  return euclidean > MOVED_LIMIT;
}

static void Debugger(movement_t *movement) {
  printf("IMU:Accel[%lu %%] = %lu / %lu\n",
         movement->crash.value * 100 / ACCELEROMETER_LIMIT,
         movement->crash.value, ACCELEROMETER_LIMIT);
  printf("IMU:Gyros[%lu %%] = %lu / %u\n",
         movement->fall.value * 100 / GYROSCOPE_LIMIT, movement->fall.value,
         GYROSCOPE_LIMIT);
}

static void RawDebugger(mems_t *mems) {
  printf("Acce:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
         "Gyro:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
         "Temp: %d\n\n",
         mems->accelerometer.x, mems->accelerometer.y, mems->accelerometer.z,
         mems->gyroscope.x, mems->gyroscope.y, mems->gyroscope.z,
         (int8_t)mems->temperature);
}
