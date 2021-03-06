/*
 * predictor.h
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__PREDICTOR_H_
#define INC_APP__PREDICTOR_H_

/* Includes
 * --------------------------------------------*/
#include "Nodes/BMS.h"

/* Exported types
 * --------------------------------------------*/
typedef struct {
  float capacity;
  float efficiency;
  uint32_t range;
} bms_avg_t;

/* Public functions prototype
 * --------------------------------------------*/
void PRD_Init(void);
void PRD_EstimateRange(void);
const bms_avg_t* PRD_IO_Avg(void);

#endif /* INC_APP__PREDICTOR_H_ */
