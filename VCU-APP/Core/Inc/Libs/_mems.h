/*
 * mems.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Puja
 */

#ifndef MEMS_H_
#define MEMS_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_mpu6050.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define MEMS_TIMEOUT (uint16_t) 5000 // in ms
#define GRAVITY_FORCE (float)9.8

#define MOVED_LIMIT (uint8_t)(10)
#define GYROSCOPE_LIMIT (uint8_t)(45)
#define ACCELEROMETER_LIMIT (uint32_t)(7000)

#define RAD2DEG(rad) ((rad)*180.0 / M_PI)

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} coordinate_t;

typedef struct {
	coordinate_t accelerometer;
	coordinate_t gyroscope;
	float temperature;
} mems_raw_t;

typedef struct __attribute__((packed)) {
	int8_t yaw;
	int8_t pitch;
	int8_t roll;
} gyroscope_t;

typedef struct {
	struct {
		uint8_t state;
		uint32_t value;
	} fall;
	struct {
		uint8_t state;
		int32_t value;
	} crash;
	uint8_t fallen;
} move_t;

typedef struct {
	uint8_t init;
	gyroscope_t gyro;
} drag_t;

typedef struct {
	uint8_t active;
	uint32_t tick;
	gyroscope_t gyro;
	mems_raw_t raw;
} mems_data_t;

typedef struct {
	mems_data_t d;
	move_t move;
	drag_t drag;
	MPU6050 dev;
	I2C_HandleTypeDef *pi2c;
} mems_t;

/* Exported variables
 * ----------------------------------------------------------*/
extern mems_t MEMS;

/* Public functions prototype ------------------------------------------------*/
uint8_t MEMS_Init(void);
void MEMS_DeInit(void);
void MEMS_Refresh(void);
void MEMS_Flush(void);
uint8_t MEMS_Capture(void);
uint8_t MEMS_Process(void);
void MEMS_ActivateDetector(void);
void MEMS_ResetDetector(void);

#endif /* MEMS_H_ */
