/*
 * mpu6050.h
 *
 *  Created on: Feb 19, 2016
 *      Author: Sina Darvishi
 */

#ifndef INC_DRIVERS__MPU_H_
#define INC_DRIVERS__MPU_H_

/* Includes
 * --------------------------------------------*/
#include "App/common.h"

/* Exported enums
 * --------------------------------------------*/
/**
 * @brief  MPU6050 can have 2 different slave addresses, depends on it's input
 * AD0 pin This feature allows you to use 2 different sensors with this library
 * at the same time
 */
typedef enum {
  MPU_Device_0 = 0x00, /*!< AD0 pin is set to low */
  MPU_Device_1 = 0x02  /*!< AD0 pin is set to high */
} MPU_Device;

/**
 * @brief  MPU6050 result enumeration
 */
typedef enum {
  MPUR_Ok = 0x00,          /*!< Everything OK */
  MPUR_Error,              /*!< Unknown error */
  MPUR_DeviceNotConnected, /*!< There is no device with valid slave address */
  MPUR_DeviceInvalid       /*!< Connected device with address is not MPU6050*/
} MPUR;

/**
 * @brief  Parameters for accelerometer range
 */
typedef enum {
  MPU_Accel_2G = 0x00, /*!< Range is +- 2G */
  MPU_Accel_4G = 0x01, /*!< Range is +- 4G */
  MPU_Accel_8G = 0x02, /*!< Range is +- 8G */
  MPU_Accel_16G = 0x03 /*!< Range is +- 16G */
} MPU_Accel;

/**
 * @brief  Parameters for gyroscope range
 */
typedef enum {
  MPU_Gyro_250s = 0x00,  /*!< Range is +- 250 degrees/s */
  MPU_Gyro_500s = 0x01,  /*!< Range is +- 500 degrees/s */
  MPU_Gyro_1000s = 0x02, /*!< Range is +- 1000 degrees/s */
  MPU_Gyro_2000s = 0x03  /*!< Range is +- 2000 degrees/s */
} MPU_Gyro;

/* Exported unions
 * --------------------------------------------*/
/**
 * @brief  Interrupts union and structure
 */
typedef union {
  struct {
    uint8_t DataReady : 1;    /*!< Data ready interrupt */
    uint8_t reserved2 : 2;    /*!< Reserved bits */
    uint8_t Master : 1;       /*!< Master interrupt. Not enabled with library */
    uint8_t FifoOverflow : 1; /*!< FIFO overflow interrupt. Not enabled with
                                 library */
    uint8_t reserved1 : 1;    /*!< Reserved bit */
    uint8_t MotionDetection : 1; /*!< Motion detected interrupt */
    uint8_t reserved0 : 1;       /*!< Reserved bit */
  } F;
  uint8_t Status;
} MPU_Interrupt;

/* Exported types
 * --------------------------------------------*/
/**
 * @brief  Main MPU6050 structure
 */
typedef struct {
  /* Private */
  float Gyro_Mult; /*!< Gyro corrector from raw data to "degrees/s". Only for
                      private use */
  float Acce_Mult; /*!< Accel corrector from raw data to "g". Only for private
                      use */
  /* Public */
  int16_t Accel_X; /*!< Accel value X axis */
  int16_t Accel_Y; /*!< Accel value Y axis */
  int16_t Accel_Z; /*!< Accel value Z axis */
  int16_t Gyro_X;  /*!< Gyro value X axis */
  int16_t Gyro_Y;  /*!< Gyro value Y axis */
  int16_t Gyro_Z;  /*!< Gyro value Z axis */
  float Temp;      /*!< Temp in degrees */
} MPU_Dev;

/* Public functions prototype
 * --------------------------------------------*/
/**
 * @brief  Initializes MPU6050 and I2C peripheral
 * @param  *DataStruct: Pointer to empty @ref MPU_t structure
 * @param  DeviceNumber: MPU6050 has one pin, AD0 which can be used to set
 * address of device. This feature allows you to use 2 different sensors on the
 * same board with same library. If you set AD0 pin to low, then this parameter
 * should be MPU_Device_0, but if AD0 pin is high, then you should use
 * MPU_Device_1
 *
 *          Parameter can be a value of @ref MPU_Device_t enumeration
 * @param  AccelSensitivity: Set accelerometer sensitivity. This
 * parameter can be a value of @ref MPU_Accel_t enumeration
 * @param  GyroSensitivity: Set gyroscope sensitivity. This parameter can
 * be a value of @ref MPU_Gyro_t enumeration
 * @retval Initialization status:
 *            - MPUR_t: Everything OK
 *            - Other member: in other cases
 */
MPUR MPU_Init(I2C_HandleTypeDef *I2Cx, MPU_Device DeviceNumber,
              MPU_Accel AccelSensitivity, MPU_Gyro GyroSensitivity);

/**
 * @brief  Sets gyroscope sensitivity
 * @param  *DataStruct: Pointer to @ref MPU_t structure indicating MPU6050
 * device
 * @param  GyroSensitivity: Gyro sensitivity value. This parameter can be a
 * value of @ref MPU_Gyro_t enumeration
 * @retval Member of @ref MPUR_t enumeration
 */
MPUR MPU_SetGyro(MPU_Gyro GyroSensitivity);

/**
 * @brief  Sets accelerometer sensitivity
 * @param  *DataStruct: Pointer to @ref MPU_t structure indicating MPU6050
 * device
 * @param  AccelSensitivity: Gyro sensitivity value. This parameter can
 * be a value of @ref MPU_Accel_t enumeration
 * @retval Member of @ref MPUR_t enumeration
 */
MPUR MPU_SetAccel(MPU_Accel AccelSensitivity);

/**
 * @brief  Sets output data rate
 * @param  *DataStruct: Pointer to @ref MPU_t structure indicating MPU6050
 * device
 * @param  rate: Data rate value. An 8-bit value for prescaler value
 * @retval Member of @ref MPUR_t enumeration
 */
MPUR MPU_SetDataRate(uint8_t rate);

/**
 * @brief  Reads accelerometer, gyroscope and temperature data from sensor
 * @param  *DataStruct: Pointer to @ref MPU_t structure to store data to
 * @retval Member of @ref MPUR_t:
 *            - MPUR_Ok: everything is OK
 *            - Other: in other cases
 */
MPUR MPU_ReadAll(MPU_Dev *mpu);

/**
 * @brief  Reads accelerometer data from sensor
 * @param  *DataStruct: Pointer to @ref MPU_t structure to store data to
 * @retval Member of @ref MPUR_t:
 *            - MPUR_Ok: everything is OK
 *            - Other: in other cases
 */
MPUR MPU_ReadAccel(void);

/**
 * @brief  Reads gyroscope data from sensor
 * @param  *DataStruct: Pointer to @ref MPU_t structure to store data to
 * @retval Member of @ref MPUR_t:
 *            - MPUR_Ok: everything is OK
 *            - Other: in other cases
 */
MPUR MPU_ReadGyro(void);

/**
 * @brief  Reads temperature data from sensor
 * @param  *DataStruct: Pointer to @ref MPU_t structure to store data to
 * @retval Member of @ref MPUR_t:
 *            - MPUR_Ok: everything is OK
 *            - Other: in other cases
 */
MPUR MPU_ReadTemp(void);

/**
 * @brief  Enables interrupts
 * @param  *DataStruct: Pointer to @ref MPU_t structure indicating MPU6050
 * device
 * @retval Member of @ref MPUR_t enumeration
 */
MPUR MPU_EnableInterrupts(void);

/**
 * @brief  Disables interrupts
 * @param  *DataStruct: Pointer to @ref MPU_t structure indicating MPU6050
 * device
 * @retval Member of @ref MPUR_t enumeration
 */
MPUR MPU_DisableInterrupts(void);

/**
 * @brief  Reads and clears interrupts
 * @param  *DataStruct: Pointer to @ref MPU_t structure indicating MPU6050
 * device
 * @param  *InterruptsStruct: Pointer to @ref MPU_Interrupt_t structure to
 * store status in
 * @retval Member of @ref MPUR_t enumeration
 */
MPUR MPU_ReadInterrupts(MPU_Interrupt *InterruptsStruct);

#endif /* INC_DRIVERS__MPU_H_ */
