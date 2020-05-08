/*
 * mems.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */

#ifndef GYRO_H_
#define GYRO_H_

/* Includes ------------------------------------------------------------------*/
#include "_mpu6050.h"

/* Exported constants --------------------------------------------------------*/
#define GYROSCOPE_LIMIT               (10000/10)
#define ACCELEROMETER_LIMIT           (46000/3)

/* Exported struct ------------------------------------------------------------*/
typedef struct {
  int32_t x;
  int32_t y;
  int32_t z;
} coordinate_t;

typedef struct {
  coordinate_t accelerometer;
  coordinate_t gyroscope;
} mems_t;

typedef struct {
  uint8_t fall;
  uint8_t crash;
} mems_decision_t;

/* Public functions prototype ------------------------------------------------*/
void GYRO_Init(void);
mems_t GYRO_Average(mems_t *calibrator, uint16_t sample);
mems_decision_t GYRO_Decision(mems_t *calibrator, uint16_t sample);

#endif /* GYRO_H_ */
