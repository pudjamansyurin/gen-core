/*
 * mems.c
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "_gyro.h"

/* External variables ----------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c3;

/* Private variables ----------------------------------------------------------*/
static MPU6050 mpu;

/* Public functions implementation --------------------------------------------*/
void GYRO_Init(void) {
  MPU6050_Result result;

  //  HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 1);
  //  osDelay(1000);

  do {
    LOG_StrLn("Gyroscope_Init");

    // MOSFET Control
    HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 0);
    osDelay(1000);
    HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, 1);
    osDelay(1000);

    //    HAL_GPIO_WritePin(INT_KEYLESS_PWR_GPIO_Port, INT_KEYLESS_PWR_Pin, 0);
    //    osDelay(5000);
    //    HAL_GPIO_WritePin(INT_KEYLESS_PWR_GPIO_Port, INT_KEYLESS_PWR_Pin, 1);
    //    osDelay(500);

    // module initialization
    result = MPU6050_Init(&hi2c3, &mpu, MPU6050_Device_0, MPU6050_Accelerometer_16G, MPU6050_Gyroscope_250s);

  } while (result != MPU6050_Result_Ok);
}

mems_t GYRO_Average(mems_t *calibrator, uint16_t sample) {
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
    MPU6050_ReadAll(&hi2c3, &mpu);
    // sum all value
    mems.accelerometer.x += mpu.Gyroscope_X;
    mems.accelerometer.y += mpu.Gyroscope_Y;
    mems.accelerometer.z += mpu.Gyroscope_Z;
    mems.gyroscope.x += mpu.Accelerometer_X;
    mems.gyroscope.y += mpu.Accelerometer_Y;
    mems.gyroscope.z += mpu.Accelerometer_Z;
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

mems_decision_t GYRO_Decision(mems_t *calibrator, uint16_t sample) {
  int32_t g_force;
  mems_decision_t mems_decision;
  mems_t mems;

  // get mems data
  mems = GYRO_Average(calibrator, sample);
  // calculate g-force
  g_force = sqrt(pow(mems.accelerometer.x, 2) +
      pow(mems.accelerometer.y, 2) +
      pow(mems.accelerometer.z, 2));
  mems_decision.crash = (g_force > ACCELEROMETER_LIMIT);
  // calculate movement change
  mems_decision.fall = (abs(mems.gyroscope.z) > GYROSCOPE_LIMIT);

  // for debugging
  char str[100];
  sprintf(str,
      "Accelerometer\n- X:%ld\n- Y:%ld\n- Z:%ld\n"
          "Gyroscope\n- X:%ld\n- Y:%ld\n- Z:%ld\n\n",
      mems.accelerometer.x, mems.accelerometer.y, mems.accelerometer.z,
      mems.gyroscope.x, mems.gyroscope.y, mems.gyroscope.z
      );
  LOG_Str(str);

  return mems_decision;
}
