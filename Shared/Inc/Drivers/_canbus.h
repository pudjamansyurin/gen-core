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
} canbus_tx_t;
typedef struct {
    CAN_RxHeaderTypeDef header;
    CAN_DATA data;
} canbus_rx_t;
typedef struct {
    canbus_tx_t tx;
    canbus_rx_t rx;
} canbus_t;

/* Public functions prototype ------------------------------------------------*/
void CANBUS_Init(void);
uint8_t CANBUS_Filter(void);
uint8_t CANBUS_Write(uint32_t StdId, uint32_t DLC, uint8_t RTR);
uint8_t CANBUS_Read(void);
uint16_t CANBUS_ReadID(void);
void CANBUS_TxDebugger(void);
void CANBUS_RxDebugger(void);

#endif /* CANBUS_H_ */
