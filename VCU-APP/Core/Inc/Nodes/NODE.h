/*
 * NODE.h
 *
 *  Created on: Mar 23, 2021
 *      Author: pujak
 */

#ifndef INC_NODES_NODE_H_
#define INC_NODES_NODE_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Libs/_utils.h"

/* Exported defines
 * ---------------------------------------------------------*/
#define NODE_RISE_TIME	((uint16_t)5000)

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
	uint8_t error;
	uint8_t overheat;
} node_data_t;

typedef struct {
	node_data_t d;
	void (*Init)(void);
	void (*Refresh)(void);
} node_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern node_t NODE;

/* Public functions implementation
 * --------------------------------------------*/
void NODE_Init(void);
void NODE_Refresh(void);


#endif /* INC_NODES_NODE_H_ */
