/*
 * _can.h
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja
 */

#ifndef CANBUS_H_
#define CANBUS_H_

#include "_config.h"

// object list
typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[8];
} canbus_tx_t;

typedef struct {
  CAN_RxHeaderTypeDef header;
  uint8_t data[8];
} canbus_rx_t;

typedef struct {
  canbus_tx_t tx;
  canbus_rx_t rx;
} canbus_t;

// function list
void CANBUS_Init(void);
void CANBUS_Set_Tx_Header(CAN_TxHeaderTypeDef *TxHeader, uint32_t StdId, uint32_t DLC);
uint8_t CANBUS_Write(canbus_tx_t *tx);
uint8_t CANBUS_Read(canbus_rx_t *rx);

#endif /* CANBUS_H_ */
