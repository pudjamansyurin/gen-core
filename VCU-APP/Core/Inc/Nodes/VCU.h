/*
 * VCU.h
 *
 *  Created on: May 11, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_NODES_VCU_H_
#define INC_NODES_VCU_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Libs/_gps.h"
#include "Libs/_mems.h"

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  EVG_NET_SOFT_RESET = 0,
  EVG_NET_HARD_RESET,
  EVG_REMOTE_MISSING,
  EVG_BIKE_FALLEN,
  EVG_BIKE_MOVED,
  EVG_BMS_ERROR,
  EVG_MCU_ERROR
} EVENTS_GROUP_BIT;

/* Exported types
 * --------------------------------------------*/
typedef struct {
  uint8_t error;
  uint8_t buffered;
  uint32_t uptime;
  struct {
    uint32_t independent;
    uint32_t ready;
  } tick;
} vcu_data_t;

typedef struct {
  vcu_data_t d;
} vcu_t;

/* Exported variables
 * --------------------------------------------*/
extern vcu_t VCU;

/* Public functions prototype
 * --------------------------------------------*/
void VCU_Init(void);
void VCU_Refresh(void);

uint8_t VCU_TX_Heartbeat(void);
uint8_t VCU_TX_SwitchControl(void);
uint8_t VCU_TX_Datetime(datetime_t dt);
uint8_t VCU_TX_ModeData(void);

#endif /* INC_NODES_VCU_H_ */
