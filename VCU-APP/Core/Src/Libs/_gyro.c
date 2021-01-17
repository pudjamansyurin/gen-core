/*
 * gyro.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "Libs/_gyro.h"

/* Private variables ----------------------------------------------------------*/
static gyro_t gyro;

/* Private functions prototype ------------------------------------------------*/
static void Average(mems_t *mems, uint16_t sample);
static void Convert(coordinate_t *gyro, motion_t *motion);
static void Debugger(movement_t *movement);
static void RawDebugger(mems_t *mems);

/* Public functions implementation --------------------------------------------*/
void GYRO_Init(I2C_HandleTypeDef *hi2c) {
  MPU6050_Result result;

  gyro.h.i2c = hi2c;

  do {
    printf("Gyro:Init\n");

    MX_I2C3_Init();
    GATE_GyroReset();

    // module initialization
    result = MPU6050_Init(gyro.h.i2c, &(gyro.mpu), MPU6050_Device_0, MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s);

  } while (result != MPU6050_Result_Ok);
}

void GYRO_DeInit(void) {
  GATE_GyroShutdown();
  HAL_I2C_DeInit(gyro.h.i2c);
}

void GYRO_Decision(movement_t *movement, motion_t *motion, uint16_t sample) {
  mems_t mems;
  motion_t mot;

  // get gyro data
  Average(&mems, sample);
  Convert(&(mems.gyroscope), &mot);
  memcpy(motion, &mot, sizeof(motion_t));

  // calculate g-force
  movement->crash.value = sqrt(
      pow(mems.accelerometer.x, 2) +
      pow(mems.accelerometer.y, 2) +
      pow(mems.accelerometer.z, 2)
  ) / GRAVITY_FORCE;
  movement->crash.state = (movement->crash.value > ACCELEROMETER_LIMIT );

  // calculate movement change
  movement->fall.value = sqrt(pow(abs(mot.roll), 2) + pow(abs(mot.pitch), 2));
  movement->fall.state = movement->fall.value > GYROSCOPE_LIMIT || mot.yaw < GYROSCOPE_LIMIT;

  movement->fallen = movement->crash.state || movement->fall.state;

  //  RawDebugger(&mems);
  //  Debugger(movement);
}

/* Private functions implementation --------------------------------------------*/
static void Average(mems_t *mems, uint16_t sample) {
  mems_t m = { 0 };
  MPU6050_Result status;

  // sampling
  for (uint16_t i = 0; i < sample; i++) {
    // read sensor
    do {
      status = MPU6050_ReadAll(gyro.h.i2c, &(gyro.mpu));

      if (status != MPU6050_Result_Ok)
        GYRO_Init(gyro.h.i2c);
    } while (status != MPU6050_Result_Ok);

    // sum all value
    // convert the RAW values into dps (deg/s)
    m.accelerometer.x += gyro.mpu.Gyroscope_X / gyro.mpu.Gyro_Mult;
    m.accelerometer.y += gyro.mpu.Gyroscope_Y / gyro.mpu.Gyro_Mult;
    m.accelerometer.z += gyro.mpu.Gyroscope_Z / gyro.mpu.Gyro_Mult;
    // convert the RAW values into acceleration in 'g'
    m.gyroscope.x += gyro.mpu.Accelerometer_X / gyro.mpu.Acce_Mult;
    m.gyroscope.y += gyro.mpu.Accelerometer_Y / gyro.mpu.Acce_Mult;
    m.gyroscope.z += gyro.mpu.Accelerometer_Z / gyro.mpu.Acce_Mult;
    // temperature
    m.temperature += gyro.mpu.Temperature;
  }

  // calculate the average
  m.accelerometer.x /= sample;
  m.accelerometer.y /= sample;
  m.accelerometer.z /= sample;
  m.gyroscope.x /= sample;
  m.gyroscope.y /= sample;
  m.gyroscope.z /= sample;
  m.temperature /= sample;

  // save the result
  *mems = m;
}

static void Convert(coordinate_t *gyro, motion_t *motion) {
  motion_float_t mot;

  // Calculating Roll and Pitch from the accelerometer data
  mot.yaw = sqrt(pow(gyro->x, 2) + pow(gyro->y, 2));
  mot.roll = sqrt(pow(gyro->x, 2) + pow(gyro->z, 2));
  mot.pitch = sqrt(pow(gyro->y, 2) + pow(gyro->z, 2));

  motion->yaw = mot.yaw == 0 ? 0 : RAD2DEG(atan(gyro->z / mot.yaw));
  motion->roll = mot.roll == 0 ? 0 : RAD2DEG(atan(gyro->y / mot.roll));
  motion->pitch = mot.pitch == 0 ? 0 : RAD2DEG(atan(gyro->x / mot.pitch));
}

static void Debugger(movement_t *movement) {
  printf("IMU:Accel[%lu %%] = %lu / %lu\n",
      movement->crash.value * 100 / ACCELEROMETER_LIMIT,
      movement->crash.value,
      ACCELEROMETER_LIMIT
  );
  printf("IMU:Gyros[%lu %%] = %lu / %u\n",
      movement->fall.value * 100 / GYROSCOPE_LIMIT,
      movement->fall.value,
      GYROSCOPE_LIMIT
  );
}

static void RawDebugger(mems_t *mems) {
  printf(
      "Accelerometer:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
      "Gyroscope:\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
      "Temperature: %d\n\n",
      mems->accelerometer.x, mems->accelerometer.y, mems->accelerometer.z,
      mems->gyroscope.x, mems->gyroscope.y, mems->gyroscope.z,
      (int8_t) mems->temperature
  );
}
