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

/* Defines -----------------------------------------------------------------*/
#define CAN_DATA 	UNION64
#define CAN_RX_MS ((uint16_t)1000)

/* Exported defines ----------------------------------------------------------*/
typedef enum {
  CAND_FOCAN_PROGRESS = 0x01A,
  CAND_FOCAN_CRC = 0x01B,
  CAND_FOCAN_PRA = 0x01C,
  CAND_FOCAN_INIT = 0x01D,
  CAND_FOCAN_RUN = 0x01E,
  CAND_FOCAN_PASCA = 0x01F,

  CAND_VCU_SWITCH_CTL = 0x02A,
  CAND_VCU_MODE_DATA = 0x02B,
  CAND_VCU_DATETIME = 0x02C,

  CAND_VCU = 0x7B0,
  CAND_HMI1 = 0x7C0,
  CAND_HMI2 = 0x7D0,

  CAND_DBG_MCU_1 = 0x7EE,
  CAND_DBG_MCU_2 = 0x7EF,
  CAND_DBG_GROUP = 0x7F0,
  CAND_DBG_VCU = 0x7F1,
  CAND_DBG_GPS_1 = 0x7F2,
  CAND_DBG_GPS_2 = 0x7F3,
  CAND_DBG_MEMS_1 = 0x7F4,
  CAND_DBG_MEMS_2 = 0x7F5,
  CAND_DBG_MEMS_3 = 0x7F6,
  CAND_DBG_MEMS_4 = 0x7F7,
  CAND_DBG_RMT_1 = 0x7F8,
  CAND_DBG_RMT_2 = 0x7F9,
  CAND_DBG_TASK_1 = 0x7FA,
  CAND_DBG_TASK_2 = 0x7FB,
  CAND_DBG_TASK_3 = 0x7FC,
  CAND_DBG_TASK_4 = 0x7FD,
  CAND_DBG_TASK_5 = 0x7FE,
  CAND_NODE_DEBUG = 0x7FF,

  CAND_MCU_MODULE_TEMP = 0x0A0,
  CAND_MCU_TORQUE_SPEED = 0x0A1,
  CAND_MCU_CURRENT_DC = 0x0A2,
  CAND_MCU_CURRENT_INFO = 0x0A3,
  CAND_MCU_FAULT_CODE = 0x0A4,
  CAND_MCU_VOLTAGE_DC = 0x0A5,
  CAND_MCU_STATE = 0x0A6,

  CAND_MCU_ERROR_1 = 0x0A7,
  CAND_MCU_ERROR_2 = 0x0A8,
  CAND_MCU_ERROR_3 = 0x0AB,
  CAND_MCU_ERROR_4 = 0x0AC,
  CAND_MCU_ERROR_5 = 0x0AD,
  CAND_MCU_ERROR_6 = 0x0AF,

  CAND_MCU_SETTING = 0x0C0,
  CAND_MCU_TEMPLATE_W = 0x0C1,
  CAND_MCU_TEMPLATE_R = 0x0C2,

  CAND_BMS_SETTING = 0x000001B2,
  CAND_BMS_PARAM_1 = 0x0B0FFFFF,
  CAND_BMS_PARAM_2 = 0x0B1FFFFF,

  CAND_BMS_TO_CHARGER = 0x0E0FFFFF,
  CAND_BMS_CODE = 0x0E1FFFFF,
  CAND_BMS_FROM_CHARGER = 0x0E3FFFFF,
  CAND_BMS_CELL_1 = 0x0B4,
  CAND_BMS_CELL_2 = 0x0B5,
  CAND_BMS_CELL_3 = 0x0B6,
  CAND_BMS_CELL_4 = 0x0B7,
  CAND_BMS_RATED = 0x0BA,

} CAN_ADDR;

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
  CAN_TxHeaderTypeDef header;
  UNION64 data;
} can_tx_t;

typedef struct {
  CAN_RxHeaderTypeDef header;
  UNION64 data;
} can_rx_t;

typedef struct {
  CAN_HandleTypeDef *pcan;
} can_t;

/* Public functions prototype ------------------------------------------------*/
void CANBUS_Init(void);
void CANBUS_DeInit(void);
uint8_t CANBUS_Filter(void);
uint8_t CANBUS_Write(can_tx_t *Tx, uint32_t address, uint32_t DLC, uint8_t ext);
uint8_t CANBUS_Read(can_rx_t *Rx);
uint32_t CANBUS_ReadID(CAN_RxHeaderTypeDef *RxHeader);

#endif /* CANBUS_H_ */
