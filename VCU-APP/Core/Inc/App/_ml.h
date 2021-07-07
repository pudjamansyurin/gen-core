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

/* Exported types
 * --------------------------------------------*/
typedef struct {
  float capacity;
  float efficiency;
  uint32_t distance;
} bms_avg_t;

/* Public functions prototype
 * --------------------------------------------*/
void ML_BMS_Init(void);
void ML_PredictRange(void);
bms_avg_t ML_IO_GetDataBMS(void);

#endif /* INC_APP__ML_H_ */
