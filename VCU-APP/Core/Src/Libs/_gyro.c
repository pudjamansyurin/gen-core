/*
 * gyro.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "Drivers/_mpu6050.h"
#include "Libs/_gyro.h"

/* Private variables ----------------------------------------------------------*/
static gyro_t gyro;
static MPU6050 mpu;

/* Private functions prototype ------------------------------------------------*/
static void Average(mems_t *mems, uint16_t sample);
static void Convert(coordinate_t *gyro, motion_t *motion);
//static void Gyro_RawDebugger(mems_t *mems);

/* Public functions implementation --------------------------------------------*/
void GYRO_Init(I2C_HandleTypeDef *hi2c) {
  MPU6050_Result result;

  gyro.handle.i2c = hi2c;

  do {
    LOG_StrLn("Gyro:Init");

    MX_I2C3_Init();
    GATE_GyroReset();

    // module initialization
    result = MPU6050_Init(gyro.handle.i2c, &mpu, MPU6050_Device_0, MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s);

  } while (result != MPU6050_Result_Ok);
}

void GYRO_DeInit(void) {
  GATE_GyroShutdown();
  MX_I2C3_DeInit();
}

movement_t GYRO_Decision(uint16_t sample, motion_t *motion) {
  mems_t mems;
  motion_t mot;
  movement_t movement;

  // get gyro data
  Average(&mems, sample);
  Convert(&(mems.gyroscope), &mot);
  memcpy(motion, &mot, sizeof(motion_t));

  // calculate g-force
  movement.crash.value = sqrt(
      pow(mems.accelerometer.x, 2) +
          pow(mems.accelerometer.y, 2) +
          pow(mems.accelerometer.z, 2)
              ) / GRAVITY_FORCE;
  movement.crash.state = (movement.crash.value > ACCELEROMETER_LIMIT );

  // calculate movement change
  movement.fall.value = sqrt(pow(abs(mot.roll), 2) + pow(abs(mot.pitch), 2));
  movement.fall.state = movement.fall.value > GYROSCOPE_LIMIT || mot.yaw < GYROSCOPE_LIMIT;

  movement.fallen = movement.crash.state || movement.fall.state;

  //	Gyro_RawDebugger(&gyro);
  return movement;
}

void GYRO_Debugger(movement_t *movement) {
  // calculated data
  LOG_Str("IMU:Accel[");
  LOG_Int(movement->crash.value * 100 / ACCELEROMETER_LIMIT);
  LOG_Str(" %] = ");
  LOG_Int(movement->crash.value);
  LOG_Str(" / ");
  LOG_Int(ACCELEROMETER_LIMIT);
  LOG_Enter();
  LOG_Str("IMU:Gyros[");
  LOG_Int(movement->fall.value * 100 / GYROSCOPE_LIMIT);
  LOG_Str(" %] = ");
  LOG_Int(movement->fall.value);
  LOG_Str(" / ");
  LOG_Int(GYROSCOPE_LIMIT);
  LOG_Enter();
}

/* Private functions implementation --------------------------------------------*/
static void Average(mems_t *mems, uint16_t sample) {
  mems_t m = { 0 };
  MPU6050_Result status;

  // sampling
  for (uint16_t i = 0; i < sample; i++) {
    // read sensor
    do {
      status = MPU6050_ReadAll(gyro.handle.i2c, &mpu);

      if (status != MPU6050_Result_Ok)
        GYRO_Init(gyro.handle.i2c);
    } while (status != MPU6050_Result_Ok);

    // sum all value
    // convert the RAW values into dps (deg/s)
    m.accelerometer.x += mpu.Gyroscope_X / mpu.Gyro_Mult;
    m.accelerometer.y += mpu.Gyroscope_Y / mpu.Gyro_Mult;
    m.accelerometer.z += mpu.Gyroscope_Z / mpu.Gyro_Mult;
    // convert the RAW values into acceleration in 'g'
    m.gyroscope.x += mpu.Accelerometer_X / mpu.Acce_Mult;
    m.gyroscope.y += mpu.Accelerometer_Y / mpu.Acce_Mult;
    m.gyroscope.z += mpu.Accelerometer_Z / mpu.Acce_Mult;
    // temperature
    m.temperature += mpu.Temperature;
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

//static void Gyro_RawDebugger(mems_t *mems) {
//	// raw data
//	char str[100];
//	sprintf(str,
//			"Accelerometer\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
//			"Gyroscope\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
//			"Temperature: %d\n\n",
//			mems->accelerometer.x, mems->accelerometer.y, mems->accelerometer.z,
//			mems->gyroscope.x, mems->gyroscope.y, mems->gyroscope.z,
//			(int8_t) mems->temperature
//	);
//	LOG_Str(str);
//}
