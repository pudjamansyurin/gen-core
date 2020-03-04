/*
 * _can.h
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja
 */

#ifndef CANBUS_H_
#define CANBUS_H_

#include "main.h"
#include "cmsis_os.h"
#include "_config.h"
#include "_database.h"

// object list
typedef struct {
	CAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];
} CANBUS_Tx;

typedef struct {
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];
} CANBUS_Rx;

// function list
void CANBUS_Init(void);
void CANBUS_Set_Tx_Header(CAN_TxHeaderTypeDef *TxHeader, uint32_t StdId, uint32_t DLC);
uint8_t CANBUS_Write(CANBUS_Tx *TxCan);
uint8_t CANBUS_Read(CANBUS_Rx *RxCan);

#endif /* CANBUS_H_ */
