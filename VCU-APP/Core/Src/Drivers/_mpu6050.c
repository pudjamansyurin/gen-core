/*
 * sd_hal_mpu6050.c
 *
 *  Created on: Feb 19, 2016
 *      Author: Sina Darvishi
 */

/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Sina Darvishi,2016
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_mpu6050.h"

/* Private variables
 * ----------------------------------------------------------*/
static uint8_t MPU_ADDRESS; /*!< I2C address of device. */
static I2C_HandleTypeDef *I2C_HANDLE;

/* Public functions declarations
 * ---------------------------------------------*/
static uint8_t I2C_Write(uint16_t MemAddress, uint8_t *pData);
static uint8_t I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size);

/* Public functions implementation
 * ---------------------------------------------*/
MPU6050_Result MPU6050_Init(I2C_HandleTypeDef *I2Cx, MPU6050 *DataStruct,
                            MPU6050_Device DeviceNumber,
                            MPU6050_Accelerometer AccelerometerSensitivity,
                            MPU6050_Gyroscope GyroscopeSensitivity) {
  uint8_t data, temp;

  /* Format I2C address */
	MPU_ADDRESS = MPU6050_I2C_ADDR | (uint8_t)DeviceNumber;
	I2C_HANDLE = I2Cx;

  /* Check if device is connected */
  if (HAL_I2C_IsDeviceReady(I2C_HANDLE, MPU_ADDRESS, 10, 1000) != HAL_OK)
    return MPU6050_Result_Error;

  /* Check who am I */
  //------------------
  /* Send address */
  if (!I2C_Read(MPU6050_WHO_AM_I, &temp, 1))
    return MPU6050_Result_Error;

  /* Checking */
  if (temp != MPU6050_I_AM)
    return MPU6050_Result_DeviceInvalid;

  /* Wakeup MPU6050 */
  data = 0x01;
  if (!I2C_Write(MPU6050_PWR_MGMT_1, &data))
    return MPU6050_Result_Error;

  /* Set sample rate to 1kHz */
  if (MPU6050_SetDataRate(MPU6050_DataRate_8KHz) != MPU6050_Result_Ok)
    return MPU6050_Result_Error;

  /* Config accelerometer */
  if (MPU6050_SetAccelerometer(DataStruct, AccelerometerSensitivity) != MPU6050_Result_Ok)
    return MPU6050_Result_Error;

  /* Config Gyroscope */
  if (MPU6050_SetGyroscope(DataStruct, GyroscopeSensitivity) != MPU6050_Result_Ok)
    return MPU6050_Result_Error;

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_SetDataRate(uint8_t rate) {
  if (!I2C_Write(MPU6050_SMPLRT_DIV, &rate))
    return MPU6050_Result_Error;
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_SetAccelerometer(MPU6050 *DataStruct, MPU6050_Accelerometer AccelerometerSensitivity) {
  uint8_t data;

  /* Config accelerometer */
  if (!I2C_Read(MPU6050_ACCEL_CONFIG, &data, 1))
    return MPU6050_Result_Error;

  data = (data & 0xE7) | (uint8_t)AccelerometerSensitivity << 3;
  if (!I2C_Write(MPU6050_ACCEL_CONFIG, &data))
    return MPU6050_Result_Error;

  /* Set sensitivities for multiplying gyro and accelerometer data */
  switch (AccelerometerSensitivity) {
  case MPU6050_Accelerometer_2G:
    DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_2;
    break;
  case MPU6050_Accelerometer_4G:
    DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_4;
    break;
  case MPU6050_Accelerometer_8G:
    DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_8;
    break;
  case MPU6050_Accelerometer_16G:
    DataStruct->Acce_Mult = (float)1 / MPU6050_ACCE_SENS_16;
    break;
  default:
    break;
  }

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_SetGyroscope(MPU6050 *DataStruct, MPU6050_Gyroscope GyroscopeSensitivity) {
  uint8_t data;

  /* Config gyroscope */
  if (!I2C_Read(MPU6050_GYRO_CONFIG, &data, 1))
    return MPU6050_Result_Error;

  data = (data & 0xE7) | (uint8_t)GyroscopeSensitivity << 3;
  if (!I2C_Write(MPU6050_GYRO_CONFIG, &data))
    return MPU6050_Result_Error;

  switch (GyroscopeSensitivity) {
  case MPU6050_Gyroscope_250s:
    DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_250;
    break;
  case MPU6050_Gyroscope_500s:
    DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_500;
    break;
  case MPU6050_Gyroscope_1000s:
    DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_1000;
    break;
  case MPU6050_Gyroscope_2000s:
    DataStruct->Gyro_Mult = (float)1 / MPU6050_GYRO_SENS_2000;
    break;
  default:
    break;
  }

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadAll(MPU6050 *DataStruct) {
  uint8_t data[14];
  int16_t temp;

  /* Read full raw data, 14bytes */
  if (!I2C_Read(MPU6050_ACCEL_XOUT_H, data, 14))
    return MPU6050_Result_Error;

  /* Format accelerometer data */
  DataStruct->Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
  DataStruct->Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
  DataStruct->Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

  /* Format temperature */
  temp = (data[6] << 8 | data[7]);
  DataStruct->Temperature =
      (float)((float)((int16_t)temp) / (float)340.0 + (float)36.53);

  /* Format gyroscope data */
  DataStruct->Gyroscope_X = (int16_t)(data[8] << 8 | data[9]);
  DataStruct->Gyroscope_Y = (int16_t)(data[10] << 8 | data[11]);
  DataStruct->Gyroscope_Z = (int16_t)(data[12] << 8 | data[13]);

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadAccelerometer(MPU6050 *DataStruct) {
  uint8_t data[6];

  /* Read accelerometer data */
  if (!I2C_Read(MPU6050_ACCEL_XOUT_H, data, 6))
    return MPU6050_Result_Error;

  /* Format */
  DataStruct->Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
  DataStruct->Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
  DataStruct->Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadGyroscope(MPU6050 *DataStruct) {
  uint8_t data[6];

  /* Read gyroscope data */
  if (!I2C_Read(MPU6050_GYRO_XOUT_H, data, 6))
    return MPU6050_Result_Error;

  /* Format */
  DataStruct->Gyroscope_X = (int16_t)(data[0] << 8 | data[1]);
  DataStruct->Gyroscope_Y = (int16_t)(data[2] << 8 | data[3]);
  DataStruct->Gyroscope_Z = (int16_t)(data[4] << 8 | data[5]);

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadTemperature(MPU6050 *DataStruct) {
  uint8_t data[2];
  int16_t temp;

  /* Read temperature */
  if (!I2C_Read(MPU6050_TEMP_OUT_H, data, 2))
    return MPU6050_Result_Error;

  /* Format temperature */
  temp = (data[0] << 8 | data[1]);
  DataStruct->Temperature =
      (float)((int16_t)temp / (float)340.0 + (float)36.53);

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_EnableInterrupts(void) {
  uint8_t temp, data;

  /* Enable interrupts for data ready and motion detect */
  data = 0x21;
  if (!I2C_Write(MPU6050_INT_ENABLE, &data))
    return MPU6050_Result_Error;

  /* Clear IRQ flag on any read operation */
  if (!I2C_Read(MPU6050_INT_PIN_CFG, &temp, 14))
    return MPU6050_Result_Error;

  temp |= 0x10;
  if (!I2C_Write(MPU6050_INT_PIN_CFG, &temp))
    return MPU6050_Result_Error;

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_DisableInterrupts(void) {
  uint8_t data;

  /* Disable interrupts */
  data = 0x00;
  if (!I2C_Write(MPU6050_INT_ENABLE, &data))
    return MPU6050_Result_Error;

  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadInterrupts(MPU6050_Interrupt *InterruptsStruct) {
  uint8_t data;

  /* Reset structure */
  InterruptsStruct->Status = 0;

  if (!I2C_Read(MPU6050_INT_STATUS, &data, 14))
    return MPU6050_Result_Error;

  /* Fill value */
  InterruptsStruct->Status = data;

  return MPU6050_Result_Ok;
}



/* Private functions implementation
 * ---------------------------------------------*/
static uint8_t I2C_Write(uint16_t MemAddress, uint8_t *pData)
{
  return (HAL_I2C_Mem_Write(I2C_HANDLE, MPU_ADDRESS, MemAddress, 1, pData, 1, 1000) ==   HAL_OK);
}

static uint8_t I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size)
{
	return (HAL_I2C_Mem_Read(I2C_HANDLE, MPU_ADDRESS, MemAddress, 1, pData, Size, 1000) == HAL_OK);
}
