/*
 * _can.h
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja
 */

#ifndef CANBUS_H_
#define CANBUS_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

#define CAN_DATA 	UNION64

/* Exported struct ------------------------------------------------------------*/
typedef struct {
    CAN_TxHeaderTypeDef header;
    UNION64 data;
} can_tx_t;

typedef struct {
    CAN_RxHeaderTypeDef header;
    UNION64 data;
} can_rx_t;

typedef struct {
  uint8_t active;
  struct {
    CAN_HandleTypeDef *can;
  } h;
} can_t;

/* Public functions prototype ------------------------------------------------*/
void CANBUS_Init(CAN_HandleTypeDef *hcan);
void CANBUS_DeInit(void);
uint8_t CANBUS_Filter(void);
uint8_t CANBUS_Write(uint32_t address, CAN_DATA *TxData, uint32_t DLC);
uint8_t CANBUS_Read(can_rx_t *Rx);
uint32_t CANBUS_ReadID(CAN_RxHeaderTypeDef *RxHeader);

#endif /* CANBUS_H_ */
