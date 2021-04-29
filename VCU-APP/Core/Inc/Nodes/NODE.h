/*
 * NODE.h
 *
 *  Created on: Mar 23, 2021
 *      Author: pujak
 */

#ifndef INC_NODES_NODE_H_
#define INC_NODES_NODE_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported defines
 * ---------------------------------------------------------*/
#define NODE_TIMEOUT_MS	((uint16_t)5000)
#define NODE_DEBUG_MS	((uint16_t)10000)

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
	uint8_t error;
	uint8_t overheat;
	uint8_t debugging;
	struct {
		uint32_t dbg;
	} tick;
	struct {
		uint8_t group;
		uint8_t vcu;
		uint8_t gps;
		uint8_t mems;
		uint8_t rmt;
		uint8_t task;
		uint8_t mcu;
	} dbg;
} node_data_t;

typedef struct {
	node_data_t d;
	struct {
		void (*DebugGroup)(void);
		void (*DebugVCU)(void);
		void (*DebugGPS)(void);
		void (*DebugMEMS)(void);
		void (*DebugRMT)(void);
		void (*DebugTASK)(void);
		void (*DebugMCU)(void);
	} t;
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

void NODE_TX_DebugGroup(void);
void NODE_TX_DebugVCU(void);
void NODE_TX_DebugGPS(void);
void NODE_TX_DebugMEMS(void);
void NODE_TX_DebugRMT(void);
void NODE_TX_DebugTASK(void);
void NODE_TX_DebugMCU(void);

#endif /* INC_NODES_NODE_H_ */
