/*
 * _can.h
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja
 */

#ifndef CAN_H_
#define CAN_H_

#include "main.h"
#include "cmsis_os.h"
#include "_config.h"
#include "_database.h"

// object list
typedef struct {
	CAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];
} CAN_Tx;

typedef struct {
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];
} CAN_Rx;

// function list
void CAN_Init(void);
void CAN_Set_Tx_Header(CAN_TxHeaderTypeDef* TxHeader, uint32_t StdId, uint32_t DLC);
HAL_StatusTypeDef CAN_Filter(void);
uint8_t CAN_Write(CAN_Tx* TxCan);
uint8_t CAN_Read(CAN_Rx* RxCan);

#endif /* CAN_H_ */
