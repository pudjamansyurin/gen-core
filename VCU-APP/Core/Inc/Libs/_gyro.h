/*
 * mems.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */

#ifndef GYRO_H_
#define GYRO_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define GRAVITY_FORCE                    (float) 9.8

#define GYROSCOPE_LIMIT                (uint8_t) (45)
#define ACCELEROMETER_LIMIT           (uint32_t) (7000)

#define RAD2DEG(rad)                             ((rad) * 180.0 / M_PI)

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	float yaw;
	float pitch;
	float roll;
} motion_t;

typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} coordinate_t;

typedef struct {
	coordinate_t accelerometer;
	coordinate_t gyroscope;
	float temperature;
} mems_t;

typedef struct {
	struct {
		uint8_t state;
		uint32_t value;
	} fall;
	struct {
		uint8_t state;
		int32_t value;
	} crash;
} mems_decision_t;

/* Public functions prototype ------------------------------------------------*/
void GYRO_Init(void);
mems_decision_t GYRO_Decision(uint16_t sample);
void Gyro_Debugger(mems_decision_t *decider);

#endif /* GYRO_H_ */
