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

/* Exported defines ----------------------------------------------------------*/
// CAN Message Address
#define CAND_VCU_SWITCH              (uint32_t) 0x000
#define CAND_VCU_DATETIME            (uint32_t) 0x001
#define CAND_VCU_SELECT_SET          (uint32_t) 0x002
#define CAND_VCU_TRIP_MODE           (uint32_t) 0x003
#define CAND_BMS_PARAM_1             (uint32_t) 0x0B0
#define CAND_BMS_PARAM_2             (uint32_t) 0x0B1
#define CAND_BMS_SETTING             (uint32_t) 0x1B2
#define CAND_HMI1                    (uint32_t) 0x7C0
#define CAND_HMI2                    (uint32_t) 0x7D0
#define CAND_SET_PROGRESS            (uint32_t) 0x101
#define CAND_GET_CHECKSUM            (uint32_t) 0x102
#define CAND_PRA_DOWNLOAD            (uint32_t) 0x103
#define CAND_INIT_DOWNLOAD           (uint32_t) 0x104
#define CAND_DOWNLOADING             (uint32_t) 0x105
#define CAND_PASCA_DOWNLOAD          (uint32_t) 0x106

#define CAN_DATA 	                    UNION64

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
