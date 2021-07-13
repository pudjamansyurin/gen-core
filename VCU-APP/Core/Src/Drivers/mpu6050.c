/*
 * mpu6050.c
 *
 *  Created on: Feb 19, 2016
 *      Author: Sina Darvishi
 */

/**
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
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/mpu6050.h"

/* Private constants
 * --------------------------------------------*/
#define MPU_I2C_ADDR 0xD0 /* Default I2C address */
#define MPU_I_AM 0x68     /* Who I am register value */

/* MPU050 registers */
#define MPU_AUX_VDDIO 0x01
#define MPU_SMPLRT_DIV 0x19
#define MPU_CONFIG 0x1A
#define MPU_GYRO_CONFIG 0x1B
#define MPU_ACCEL_CONFIG 0x1C
#define MPU_MOTION_THRESH 0x1F
#define MPU_INT_PIN_CFG 0x37
#define MPU_INT_ENABLE 0x38
#define MPU_INT_STATUS 0x3A
#define MPU_ACCEL_XOUT_H 0x3B
#define MPU_ACCEL_XOUT_L 0x3C
#define MPU_ACCEL_YOUT_H 0x3D
#define MPU_ACCEL_YOUT_L 0x3E
#define MPU_ACCEL_ZOUT_H 0x3F
#define MPU_ACCEL_ZOUT_L 0x40
#define MPU_TEMP_OUT_H 0x41
#define MPU_TEMP_OUT_L 0x42
#define MPU_GYRO_XOUT_H 0x43
#define MPU_GYRO_XOUT_L 0x44
#define MPU_GYRO_YOUT_H 0x45
#define MPU_GYRO_YOUT_L 0x46
#define MPU_GYRO_ZOUT_H 0x47
#define MPU_GYRO_ZOUT_L 0x48
#define MPU_MOT_DETECT_STATUS 0x61
#define MPU_SIGNAL_PATH_RESET 0x68
#define MPU_MOT_DETECT_CTRL 0x69
#define MPU_USER_CTRL 0x6A
#define MPU_PWR_MGMT_1 0x6B
#define MPU_PWR_MGMT_2 0x6C
#define MPU_FIFO_COUNTH 0x72
#define MPU_FIFO_COUNTL 0x73
#define MPU_FIFO_R_W 0x74
#define MPU_WHO_AM_I 0x75
/* Gyro sensitivities in degrees/s */
#define MPU_GYRO_SENS_250 ((float)131)
#define MPU_GYRO_SENS_500 ((float)65.5)
#define MPU_GYRO_SENS_1000 ((float)32.8)
#define MPU_GYRO_SENS_2000 ((float)16.4)
/* Acce sensitivities in g/s */
#define MPU_ACCE_SENS_2 ((float)16384)
#define MPU_ACCE_SENS_4 ((float)8192)
#define MPU_ACCE_SENS_8 ((float)4096)
#define MPU_ACCE_SENS_16 ((float)2048)
/**
 * @brief  Data rates predefined constants
 */
#define MPU_DataRate_8KHz 0   /*!< Sample rate set to 8 kHz */
#define MPU_DataRate_4KHz 1   /*!< Sample rate set to 4 kHz */
#define MPU_DataRate_2KHz 3   /*!< Sample rate set to 2 kHz */
#define MPU_DataRate_1KHz 7   /*!< Sample rate set to 1 kHz */
#define MPU_DataRate_500Hz 15 /*!< Sample rate set to 500 Hz */
#define MPU_DataRate_250Hz 31 /*!< Sample rate set to 250 Hz */
#define MPU_DataRate_125Hz 63 /*!< Sample rate set to 125 Hz */
#define MPU_DataRate_100Hz 79 /*!< Sample rate set to 100 Hz */

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint8_t address;
  MPU_Dev dev;
  I2C_HandleTypeDef *pi2c;
} mpu6050_t;

/* Private variables
 * --------------------------------------------*/
static mpu6050_t MP;

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t I2C_Write(uint16_t MemAddress, uint8_t *pData);
static uint8_t I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size);

/* Public functions implementation
 * --------------------------------------------*/
MPUR MPU_Init(I2C_HandleTypeDef *I2Cx, MPU_Device DeviceNumber,
              MPU_Accel AccelSensitivity, MPU_Gyro GyroSensitivity) {
  uint8_t data, temp;

  /* Format I2C address */
  MP.address = MPU_I2C_ADDR | (uint8_t)DeviceNumber;
  MP.pi2c = I2Cx;

  /* Check if device is connected */
  if (HAL_I2C_IsDeviceReady(MP.pi2c, MP.address, 10, 100) != HAL_OK)
    return MPUR_Error;

  /* Check who am I */
  /* Send address */
  if (!I2C_Read(MPU_WHO_AM_I, &temp, 1)) return MPUR_Error;

  /* Checking */
  if (temp != MPU_I_AM) return MPUR_DeviceInvalid;

  /* Wakeup MPU050 */
  data = 0x01;
  if (!I2C_Write(MPU_PWR_MGMT_1, &data)) return MPUR_Error;

  /* Set sample rate to 1kHz */
  if (MPU_SetDataRate(MPU_DataRate_8KHz) != MPUR_Ok) return MPUR_Error;

  /* Config accelerometer */
  if (MPU_SetAccel(AccelSensitivity) != MPUR_Ok) return MPUR_Error;

  /* Config Gyro */
  if (MPU_SetGyro(GyroSensitivity) != MPUR_Ok) return MPUR_Error;

  /* Return OK */
  return MPUR_Ok;
}

MPUR MPU_SetDataRate(uint8_t rate) {
  if (!I2C_Write(MPU_SMPLRT_DIV, &rate)) return MPUR_Error;
  return MPUR_Ok;
}

MPUR MPU_SetAccel(MPU_Accel AccelSensitivity) {
  uint8_t data;

  /* Config accelerometer */
  if (!I2C_Read(MPU_ACCEL_CONFIG, &data, 1)) return MPUR_Error;

  data = (data & 0xE7) | (uint8_t)AccelSensitivity << 3;
  if (!I2C_Write(MPU_ACCEL_CONFIG, &data)) return MPUR_Error;

  /* Set sensitivities for multiplying gyro and accelerometer data */
  switch (AccelSensitivity) {
    case MPU_Accel_2G:
      MP.dev.Acce_Mult = (float)1 / MPU_ACCE_SENS_2;
      break;
    case MPU_Accel_4G:
      MP.dev.Acce_Mult = (float)1 / MPU_ACCE_SENS_4;
      break;
    case MPU_Accel_8G:
      MP.dev.Acce_Mult = (float)1 / MPU_ACCE_SENS_8;
      break;
    case MPU_Accel_16G:
      MP.dev.Acce_Mult = (float)1 / MPU_ACCE_SENS_16;
      break;
    default:
      break;
  }

  return MPUR_Ok;
}

MPUR MPU_SetGyro(MPU_Gyro GyroSensitivity) {
  uint8_t data;

  /* Config gyroscope */
  if (!I2C_Read(MPU_GYRO_CONFIG, &data, 1)) return MPUR_Error;

  data = (data & 0xE7) | (uint8_t)GyroSensitivity << 3;
  if (!I2C_Write(MPU_GYRO_CONFIG, &data)) return MPUR_Error;

  switch (GyroSensitivity) {
    case MPU_Gyro_250s:
      MP.dev.Gyro_Mult = (float)1 / MPU_GYRO_SENS_250;
      break;
    case MPU_Gyro_500s:
      MP.dev.Gyro_Mult = (float)1 / MPU_GYRO_SENS_500;
      break;
    case MPU_Gyro_1000s:
      MP.dev.Gyro_Mult = (float)1 / MPU_GYRO_SENS_1000;
      break;
    case MPU_Gyro_2000s:
      MP.dev.Gyro_Mult = (float)1 / MPU_GYRO_SENS_2000;
      break;
    default:
      break;
  }

  return MPUR_Ok;
}

MPUR MPU_ReadAll(MPU_Dev *mpu) {
  uint8_t data[14];
  int16_t temp;

  /* Read full raw data, 14bytes */
  if (!I2C_Read(MPU_ACCEL_XOUT_H, data, 14)) return MPUR_Error;

  /* Format accelerometer data */
  MP.dev.Accel_X = (int16_t)(data[0] << 8 | data[1]);
  MP.dev.Accel_Y = (int16_t)(data[2] << 8 | data[3]);
  MP.dev.Accel_Z = (int16_t)(data[4] << 8 | data[5]);

  /* Format temperature */
  temp = (data[6] << 8 | data[7]);
  MP.dev.Temp = (float)((float)((int16_t)temp) / (float)340.0 + (float)36.53);

  /* Format gyroscope data */
  MP.dev.Gyro_X = (int16_t)(data[8] << 8 | data[9]);
  MP.dev.Gyro_Y = (int16_t)(data[10] << 8 | data[11]);
  MP.dev.Gyro_Z = (int16_t)(data[12] << 8 | data[13]);

  memcpy(mpu, &(MP.dev), sizeof(MPU_Dev));
  return MPUR_Ok;
}

MPUR MPU_ReadAccel(void) {
  uint8_t data[6];

  /* Read accelerometer data */
  if (!I2C_Read(MPU_ACCEL_XOUT_H, data, 6)) return MPUR_Error;

  /* Format */
  MP.dev.Accel_X = (int16_t)(data[0] << 8 | data[1]);
  MP.dev.Accel_Y = (int16_t)(data[2] << 8 | data[3]);
  MP.dev.Accel_Z = (int16_t)(data[4] << 8 | data[5]);

  return MPUR_Ok;
}

MPUR MPU_ReadGyro(void) {
  uint8_t data[6];

  /* Read gyroscope data */
  if (!I2C_Read(MPU_GYRO_XOUT_H, data, 6)) return MPUR_Error;

  /* Format */
  MP.dev.Gyro_X = (int16_t)(data[0] << 8 | data[1]);
  MP.dev.Gyro_Y = (int16_t)(data[2] << 8 | data[3]);
  MP.dev.Gyro_Z = (int16_t)(data[4] << 8 | data[5]);

  return MPUR_Ok;
}

MPUR MPU_ReadTemp(void) {
  uint8_t data[2];
  int16_t temp;

  /* Read temperature */
  if (!I2C_Read(MPU_TEMP_OUT_H, data, 2)) return MPUR_Error;

  /* Format temperature */
  temp = (data[0] << 8 | data[1]);
  MP.dev.Temp = (float)((int16_t)temp / (float)340.0 + (float)36.53);

  return MPUR_Ok;
}

MPUR MPU_EnableInterrupts(void) {
  uint8_t temp, data;

  /* Enable interrupts for data ready and motion detect */
  data = 0x21;
  if (!I2C_Write(MPU_INT_ENABLE, &data)) return MPUR_Error;

  /* Clear IRQ flag on any read operation */
  if (!I2C_Read(MPU_INT_PIN_CFG, &temp, 14)) return MPUR_Error;

  temp |= 0x10;
  if (!I2C_Write(MPU_INT_PIN_CFG, &temp)) return MPUR_Error;

  return MPUR_Ok;
}

MPUR MPU_DisableInterrupts(void) {
  uint8_t data;

  /* Disable interrupts */
  data = 0x00;
  if (!I2C_Write(MPU_INT_ENABLE, &data)) return MPUR_Error;

  return MPUR_Ok;
}

MPUR MPU_ReadInterrupts(MPU_Interrupt *InterruptsStruct) {
  uint8_t data;

  /* Reset structure */
  InterruptsStruct->Status = 0;

  if (!I2C_Read(MPU_INT_STATUS, &data, 14)) return MPUR_Error;

  /* Fill value */
  InterruptsStruct->Status = data;

  return MPUR_Ok;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t I2C_Write(uint16_t MemAddress, uint8_t *pData) {
  return (HAL_I2C_Mem_Write(MP.pi2c, MP.address, MemAddress, 1, pData, 1,
                            1000) == HAL_OK);
}

static uint8_t I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size) {
  return (HAL_I2C_Mem_Read(MP.pi2c, MP.address, MemAddress, 1, pData, Size,
                           1000) == HAL_OK);
}
