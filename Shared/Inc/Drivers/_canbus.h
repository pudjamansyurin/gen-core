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

/* Exported union ------------------------------------------------------------*/
typedef union {
    uint8_t u8[8];
    uint16_t u16[4];
    uint32_t u32[2];
    uint64_t u64;
    int8_t s8[8];
    int16_t s16[4];
    int32_t s32[2];
    int64_t s64;
    char CHAR[8];
    float FLOAT[2];
    double DOUBLE;
} CAN_DATA;

/* Exported struct ------------------------------------------------------------*/
typedef struct {
    CAN_TxHeaderTypeDef header;
    CAN_DATA data;
} can_tx_t;

typedef struct {
    CAN_RxHeaderTypeDef header;
    CAN_DATA data;
} can_rx_t;

typedef struct {
  uint8_t active;
  struct {
    CAN_HandleTypeDef *can;
  } h;
} can_t;

/* Public functions prototype ------------------------------------------------*/
void CANBUS_Init(CAN_HandleTypeDef *hcan);
uint8_t CANBUS_Filter(void);
uint8_t CANBUS_Write(uint32_t address, CAN_DATA *TxData, uint32_t DLC);
uint8_t CANBUS_Read(can_rx_t *Rx);
uint32_t CANBUS_ReadID(CAN_RxHeaderTypeDef *RxHeader);

#endif /* CANBUS_H_ */
