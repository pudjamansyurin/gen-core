/*
 * mems.h
 *
 *  Created on: Aug 23, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__MEMS_H_
#define INC_LIBS__MEMS_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/mpu6050.h"

/* Private enums
 * --------------------------------------------*/
typedef enum {
  MTILT_NOW,
  MTILT_REFF,
  MTILT_MAX,
} MEMS_TILT;

typedef enum {
  MEFFECT_FALL,
  MEFFECT_CRASH,
  MEFFECT_MAX,
} MEMS_EFFECT;

/* Exported types
 * --------------------------------------------*/
typedef struct __attribute__((packed)) {
  float pitch;
  float roll;
} mems_tilt_t;

typedef struct {
  float x;
  float y;
  float z;
} mems_axis_t;

typedef struct {
  float accel;
  float gyro;
  float tilt;
} mems_total_t;

typedef struct {
  mems_axis_t accel;
  mems_axis_t gyro;
  float temp;
} mems_raw_t;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t MEMS_Init(void);
void MEMS_DeInit(void);
void MEMS_Refresh(void);
void MEMS_Flush(void);
uint8_t MEMS_Capture(void);
uint8_t MEMS_Process(void);
void MEMS_CaptureMotion(void);
uint8_t MEMS_Dragged(void);
void MEMS_ToggleMotion(void);

uint8_t MEMS_IO_Active(void);
uint8_t MEMS_IO_MotionActive(void);
uint8_t MEMS_IO_MotionOffset(void);
const mems_raw_t* MEMS_IO_Raw(void);
const mems_total_t* MEMS_IO_Total(void);
const mems_tilt_t* MEMS_IO_Tilt(MEMS_TILT key);
uint8_t MEMS_IO_Effect(MEMS_EFFECT key);
#endif /* INC_LIBS__MEMS_H_ */
