/*
 * _ml.h
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__ML_H_
#define INC_APP__ML_H_

/* Includes
 * --------------------------------------------*/
#include "Nodes/BMS.h"

/* Exported constants
 * --------------------------------------------*/
#define BMS_AVG_SZ ((uint8_t)10)

/* Exported structs
 * --------------------------------------------*/
typedef struct {
	float capacity;
	float efficiency;
	uint32_t distance;
} bms_prediction_t;

typedef struct {
	avg_float_t handle[BMS_AVG_MAX];
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
//extern ml_t ML;

/* Public functions prototype
 * --------------------------------------------*/
void ML_BMS_Init(void);
void ML_PredictRange(void);
bms_prediction_t ML_IO_GetDataBMS(void);

#endif /* INC_APP__ML_H_ */
