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

/* Public functions implementation
 * ---------------------------------------------*/
MPU6050_Result MPU6050_Init(I2C_HandleTypeDef *I2Cx, MPU6050 *DataStruct,
                            MPU6050_Device DeviceNumber,
                            MPU6050_Accelerometer AccelerometerSensitivity,
                            MPU6050_Gyroscope GyroscopeSensitivity) {
  uint8_t data, temp;

  /* Format I2C address */
  DataStruct->Address = MPU6050_I2C_ADDR | (uint8_t)DeviceNumber;
  uint8_t address = DataStruct->Address;

  /* Check if device is connected */
  if (HAL_I2C_IsDeviceReady(I2Cx, address, 10, 1000) != HAL_OK)
    return MPU6050_Result_Error;

  /* Check who am I */
  //------------------
  /* Send address */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_WHO_AM_I, 1, &temp, 1, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Checking */
  if (temp != MPU6050_I_AM)
    /* Return error */
    return MPU6050_Result_DeviceInvalid;

  /* Wakeup MPU6050 */
  data = 0x01;
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_PWR_MGMT_1, 1, &data, 1, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Set sample rate to 1kHz */
  if (MPU6050_SetDataRate(I2Cx, DataStruct, MPU6050_DataRate_8KHz) !=
      MPU6050_Result_Ok)
    return MPU6050_Result_Error;

  /* Config accelerometer */
  if (MPU6050_SetAccelerometer(I2Cx, DataStruct, AccelerometerSensitivity) !=
      MPU6050_Result_Ok)
    return MPU6050_Result_Error;

  /* Config Gyroscope */
  if (MPU6050_SetGyroscope(I2Cx, DataStruct, GyroscopeSensitivity) !=
      MPU6050_Result_Ok)
    return MPU6050_Result_Error;

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_SetDataRate(I2C_HandleTypeDef *I2Cx, MPU6050 *DataStruct,
                                   uint8_t rate) {
  uint8_t address = DataStruct->Address;

  /* Set data sample rate */
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_SMPLRT_DIV, 1, &rate, 1, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result
MPU6050_SetAccelerometer(I2C_HandleTypeDef *I2Cx, MPU6050 *DataStruct,
                         MPU6050_Accelerometer AccelerometerSensitivity) {
  uint8_t data, address = DataStruct->Address;

  /* Config accelerometer */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_ACCEL_CONFIG, 1, &data, 1,
                       1000) != HAL_OK)
    return MPU6050_Result_Error;

  data = (data & 0xE7) | (uint8_t)AccelerometerSensitivity << 3;
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_ACCEL_CONFIG, 1, &data, 1,
                        1000) != HAL_OK)
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

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_SetGyroscope(I2C_HandleTypeDef *I2Cx,
                                    MPU6050 *DataStruct,
                                    MPU6050_Gyroscope GyroscopeSensitivity) {
  uint8_t data, address = DataStruct->Address;

  /* Config gyroscope */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_GYRO_CONFIG, 1, &data, 1, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  data = (data & 0xE7) | (uint8_t)GyroscopeSensitivity << 3;
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_GYRO_CONFIG, 1, &data, 1,
                        1000) != HAL_OK)
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
  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadAll(I2C_HandleTypeDef *I2Cx, MPU6050 *DataStruct) {
  uint8_t data[14], address = DataStruct->Address;
  int16_t temp;

  /* Read full raw data, 14bytes */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_ACCEL_XOUT_H, 1, data, 14,
                       1000) != HAL_OK)
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

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadAccelerometer(I2C_HandleTypeDef *I2Cx,
                                         MPU6050 *DataStruct) {
  uint8_t data[6], address = DataStruct->Address;

  /* Read accelerometer data */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_ACCEL_XOUT_H, 1, data, 6, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Format */
  DataStruct->Accelerometer_X = (int16_t)(data[0] << 8 | data[1]);
  DataStruct->Accelerometer_Y = (int16_t)(data[2] << 8 | data[3]);
  DataStruct->Accelerometer_Z = (int16_t)(data[4] << 8 | data[5]);

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadGyroscope(I2C_HandleTypeDef *I2Cx,
                                     MPU6050 *DataStruct) {
  uint8_t data[6], address = DataStruct->Address;

  /* Read gyroscope data */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_GYRO_XOUT_H, 1, data, 6, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Format */
  DataStruct->Gyroscope_X = (int16_t)(data[0] << 8 | data[1]);
  DataStruct->Gyroscope_Y = (int16_t)(data[2] << 8 | data[3]);
  DataStruct->Gyroscope_Z = (int16_t)(data[4] << 8 | data[5]);

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadTemperature(I2C_HandleTypeDef *I2Cx,
                                       MPU6050 *DataStruct) {
  uint8_t data[2], address = DataStruct->Address;
  int16_t temp;

  /* Read temperature */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_TEMP_OUT_H, 1, data, 2, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Format temperature */
  temp = (data[0] << 8 | data[1]);
  DataStruct->Temperature =
      (float)((int16_t)temp / (float)340.0 + (float)36.53);

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_EnableInterrupts(I2C_HandleTypeDef *I2Cx,
                                        MPU6050 *DataStruct) {
  uint8_t temp, data, address = DataStruct->Address;

  /* Enable interrupts for data ready and motion detect */
  data = 0x21;
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_INT_ENABLE, 1, &data, 1, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Clear IRQ flag on any read operation */
  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_INT_PIN_CFG, 1, &temp, 14,
                       1000) != HAL_OK)
    return MPU6050_Result_Error;

  temp |= 0x10;
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_INT_PIN_CFG, 1, &temp, 1,
                        1000) != HAL_OK)
    return MPU6050_Result_Error;

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_DisableInterrupts(I2C_HandleTypeDef *I2Cx,
                                         MPU6050 *DataStruct) {
  uint8_t data, address = DataStruct->Address;

  /* Disable interrupts */
  data = 0x00;
  if (HAL_I2C_Mem_Write(I2Cx, address, MPU6050_INT_ENABLE, 1, &data, 1, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Return OK */
  return MPU6050_Result_Ok;
}

MPU6050_Result MPU6050_ReadInterrupts(I2C_HandleTypeDef *I2Cx,
                                      MPU6050 *DataStruct,
                                      MPU6050_Interrupt *InterruptsStruct) {
  uint8_t data, address = DataStruct->Address;

  /* Reset structure */
  InterruptsStruct->Status = 0;

  if (HAL_I2C_Mem_Read(I2Cx, address, MPU6050_INT_STATUS, 1, &data, 14, 1000) !=
      HAL_OK)
    return MPU6050_Result_Error;

  /* Fill value */
  InterruptsStruct->Status = data;

  /* Return OK */
  return MPU6050_Result_Ok;
}
