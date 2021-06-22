/*
 * _ml.h
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_BUSINESS__ML_H_
#define INC_BUSINESS__ML_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_utils.h"
#include "Nodes/BMS.h"

/* Exported constants
 * --------------------------------------------*/
#define BMS_AVG_SZ ((uint8_t)10)

/* Exported struct
 * --------------------------------------------*/
typedef struct {
  float capacity;
  float efficiency;
  uint32_t distance;
} bms_prediction_t;

typedef struct {
  averager_float_t handle[BMS_AVG_MAX];
  float buffer[BMS_AVG_MAX][BMS_AVG_SZ];
} bms_avg_t;

typedef struct {
  struct {
    bms_prediction_t d;
    bms_avg_t avg;
  } bms;
} ml_t;

/* Exported variables
 * --------------------------------------------*/
extern ml_t ML;

/* Public functions prototype
 * --------------------------------------------*/
void ML_BMS_Init(void);
void ML_PredictRange(void);

#endif /* INC_BUSINESS__ML_H_ */
