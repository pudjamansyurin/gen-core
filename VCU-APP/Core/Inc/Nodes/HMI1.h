/*
 * HMI1.h
 *
 *  Created on: May 10, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_NODES_HMI1_H_
#define INC_NODES_HMI1_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/can.h"

/* Exported constants
 * --------------------------------------------*/
#define HMI1_TIMEOUT_MS ((uint32_t)10000)

/* Exported types
 * --------------------------------------------*/
typedef struct {
  uint8_t active;
  uint32_t tick;
  uint16_t version;
} hmi1_data_t;

typedef struct {
  hmi1_data_t d;
} hmi1_t;

/* Exported variables
 * --------------------------------------------*/
extern hmi1_t HMI1;

/* Public functions prototype
 * --------------------------------------------*/
void HMI1_Init(void);
void HMI1_Refresh(void);
void HMI1_Power(uint8_t state);
void HMI1_RX_State(can_rx_t *Rx);

#endif /* INC_NODES_HMI1_H_ */
