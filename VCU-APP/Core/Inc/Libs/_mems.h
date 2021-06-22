/*
 * mems.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */

#ifndef MEMS_H_
#define MEMS_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_mpu6050.h"
#include "Libs/_utils.h"

/* Exported constants
 * --------------------------------------------*/
#define MEMS_AVG_SZ ((uint8_t)10)
#define MEMS_TIMEOUT_MS ((uint16_t)5000)

#define DRAGGED_LIMIT ((uint8_t)5)
#define FALL_LIMIT ((uint8_t)60)
#define CRASH_LIMIT ((uint8_t)20)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  MEMS_AVG_ACCEL = 0,
  MEMS_AVG_GYRO,
  MEMS_AVG_TILT,
  MEMS_AVG_MAX,
} MEMS_AVG_TYPE;

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  float x;
  float y;
  float z;
} mems_axis_t;

typedef struct __attribute__((packed)) {
  float pitch;
  float roll;
} mems_tilt_t;

typedef struct {
  mems_axis_t accel;
  mems_axis_t gyro;
  float temp;
} mems_raw_t;

typedef struct {
  float accel;
  float gyro;
  float tilt;
} mems_total_t;

typedef struct {
  uint8_t active;
  uint8_t offset;
  struct {
    mems_tilt_t cur;
    mems_tilt_t ref;
  } tilt;
} mems_detector_t;

typedef struct {
  uint8_t active;
  uint32_t tick;
  uint8_t fall;
  uint8_t crash;
  mems_raw_t raw;
  mems_total_t tot;
} mems_data_t;

typedef struct {
  averager_float_t handle[MEMS_AVG_MAX];
  float buffer[MEMS_AVG_MAX][MEMS_AVG_SZ];
} mems_avg_t;

typedef struct {
  mems_data_t d;
  mems_detector_t det;
  mems_avg_t avg;
  MPU6050 dev;
  I2C_HandleTypeDef *pi2c;
} mems_t;

/* Exported variables
 * --------------------------------------------*/
extern mems_t MEMS;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t MEMS_Init(void);
void MEMS_DeInit(void);
void MEMS_Refresh(void);
void MEMS_Flush(void);
uint8_t MEMS_Capture(void);
uint8_t MEMS_Process(void);
void MEMS_GetRefDetector(void);
uint8_t MEMS_Dragged(void);

#endif /* MEMS_H_ */
