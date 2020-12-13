/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Libs/_gyro.h"
#include "Drivers/_mpu6050.h"
#include "i2c.h"

/* External variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c3;

/* Private variables ----------------------------------------------------------*/
static MPU6050 mpu;

/* Private functions prototype ------------------------------------------------*/
static void Average(mems_t *mems, uint16_t sample);
static void Convert(coordinate_t *gyro, motion_float_t *motion);
//static void Gyro_RawDebugger(mems_t *mems);

/* Public functions implementation --------------------------------------------*/
void GYRO_Init(void) {
  MPU6050_Result result;

  do {
    LOG_StrLn("Gyro:Init");

    // MOSFET Control
    HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 0);
    _DelayMS(500);
    HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 1);
    _DelayMS(500);

    // Reset peripheral
    MX_I2C3_DeInit();
    MX_I2C3_Init();

    // module initialization
    result = MPU6050_Init(&hi2c3, &mpu, MPU6050_Device_0, MPU6050_Accelerometer_16G, MPU6050_Gyroscope_2000s);

  } while (result != MPU6050_Result_Ok);
}

mems_decision_t GYRO_Decision(uint16_t sample, motion_t *motion) {
  mems_t mems;
  motion_float_t mot;
  mems_decision_t decider;

  // get mems data
  Average(&mems, sample);
  Convert(&(mems.gyroscope), &mot);

  // calculate g-force
  decider.crash.value = sqrt(
      pow(mems.accelerometer.x, 2) +
          pow(mems.accelerometer.y, 2) +
          pow(mems.accelerometer.z, 2)
              ) / GRAVITY_FORCE;
  decider.crash.state = (decider.crash.value > ACCELEROMETER_LIMIT );

  // calculate movement change
  decider.fall.value = sqrt(pow(abs(mot.roll), 2) + pow(abs(mot.pitch), 2));
  decider.fall.state = decider.fall.value > GYROSCOPE_LIMIT || mot.yaw < GYROSCOPE_LIMIT;

  motion->yaw = (int8_t) mot.yaw;
  motion->roll = (int8_t) mot.roll;
  motion->pitch = (int8_t) mot.pitch;

  //	Gyro_RawDebugger(&mems);

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

/* Private functions implementation --------------------------------------------*/
static void Average(mems_t *mems, uint16_t sample) {
  mems_t m = { 0 };
  MPU6050_Result status;

  // sampling
  for (uint16_t i = 0; i < sample; i++) {
    // read sensor
    do {
      status = MPU6050_ReadAll(&hi2c3, &mpu);

      if (status != MPU6050_Result_Ok)
        GYRO_Init();
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

static void Convert(coordinate_t *gyro, motion_float_t *motion) {
  // Calculating Roll and Pitch from the accelerometer data
  motion->yaw = sqrt(pow(gyro->x, 2) + pow(gyro->y, 2));
  motion->roll = sqrt(pow(gyro->x, 2) + pow(gyro->z, 2));
  motion->pitch = sqrt(pow(gyro->y, 2) + pow(gyro->z, 2));

  motion->yaw = motion->yaw == 0 ? 0 : RAD2DEG(atan(gyro->z / motion->yaw));
  motion->roll = motion->roll == 0 ? 0 : RAD2DEG(atan(gyro->y / motion->roll));
  motion->pitch = motion->pitch == 0 ? 0 : RAD2DEG(atan(gyro->x / motion->pitch));
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
