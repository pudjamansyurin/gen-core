/*
 * NODE.h
 *
 *  Created on: Mar 23, 2021
 *      Author: pujak
 */

#ifndef INC_NODES_NODE_H_
#define INC_NODES_NODE_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Libs/_utils.h"

/* Exported constants
 * --------------------------------------------*/
#define NODE_TIMEOUT_MS ((uint16_t)5000)
#define NODE_DEBUG_MS ((uint16_t)1000)

#define CDBG_ID(c, dbg) ((c >> dbg) & 0x01)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  CDBG_GROUP = 0,
  CDBG_VCU,
  CDBG_GPS,
  CDBG_MEMS,
  CDBG_RMT,
  CDBG_TASK,
  CDBG_MCU,
  CDBG_BMS,
} CDBG;

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint8_t error;
  uint8_t overheat;
  uint8_t debug;
  struct {
    uint32_t dbg;
  } tick;
} node_data_t;

typedef struct {
  node_data_t d;
} node_t;

/* Exported variables
 * --------------------------------------------*/
extern node_t NODE;

/* Public functions prototype
 * --------------------------------------------*/
void NODE_Init(void);
void NODE_Refresh(void);
void NODE_DebugCAN(void);

void NODE_RX_Debug(can_rx_t *Rx);
void NODE_TX_DebugGroup(void);
void NODE_TX_DebugVCU(void);
void NODE_TX_DebugGPS(void);
void NODE_TX_DebugMEMS(void);
void NODE_TX_DebugRMT(void);
void NODE_TX_DebugTASK(void);
void NODE_TX_DebugMCU(void);
void NODE_TX_DebugBMS(void);

#endif /* INC_NODES_NODE_H_ */
