/*
 * HMI1.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI1_H_
#define INC_NODES_HMI1_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Drivers/_canbus.h"

/* Exported struct ------------------------------------------------------------*/
typedef struct {
  uint8_t started;
  uint32_t tick;
  uint16_t version;
  struct {
    //	uint8_t abs;
    uint8_t mirroring;
    //  uint8_t lamp;
    uint8_t warning;
    uint8_t overheat;
    uint8_t finger;
    uint8_t unremote;
    uint8_t daylight;
  } state;
} hmi1_data_t;

typedef struct {
  struct {
    void (*State)(can_rx_t*);
  } r;
} hmi1_can_t;

typedef struct {
  hmi1_data_t d;
  hmi1_can_t can;
  void (*Init)(void);
  void (*Refresh)(void);
  void (*Power)(GPIO_PinState);
} hmi1_t;

/* Public functions implementation --------------------------------------------*/
void HMI1_Init(void);
void HMI1_Refresh(void);
void HMI1_CAN_RX_State(can_rx_t *Rx);

#endif /* INC_NODES_HMI1_H_ */
