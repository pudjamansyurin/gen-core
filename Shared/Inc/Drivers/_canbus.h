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

#define CAN_DATA 	        UNION64
/* Exported defines ----------------------------------------------------------*/
typedef enum {
	CAND_FOCAN_PROGRESS 		= 0x031,
	CAND_FOCAN_CRC 					= 0x032,
	CAND_FOCAN_PRA 					= 0x033,
	CAND_FOCAN_INIT 				= 0x034,
	CAND_FOCAN_RUNNING 			= 0x035,
	CAND_FOCAN_PASCA 				= 0x036,

	CAND_VCU_SWITCH 				= 0x050,
	CAND_VCU_DATETIME 			= 0x051,
	CAND_VCU_SELECT_SET 		= 0x052,
	CAND_VCU_TRIP_MODE 			= 0x053,

	CAND_HMI1 							= 0x7C0,
	CAND_HMI2 							= 0x7D0,

	CAND_BMS_SETTING 				= 0x000001B2,
	CAND_BMS_PARAM_1 				= 0x0B0FFFFF,
	CAND_BMS_PARAM_2 				= 0x0B1FFFFF,

	CAND_BMS_TO_CHARGER 		= 0x0E0FFFFF,
	CAND_BMS_CODE 					= 0x0E1FFFFF,
	CAND_BMS_FROM_CHARGER 	= 0x0E3FFFFF,
	CAND_BMS_CELL_1 				= 0x0B4,
	CAND_BMS_CELL_2 				= 0x0B5,
	CAND_BMS_CELL_3 				= 0x0B6,
	CAND_BMS_CELL_4 				= 0x0B7,
	CAND_BMS_RATED 					= 0x0BA,

	CAND_MCU_MESSAGE				= 0x0C0,
	CAND_MCU_RW_PARAM       = 0x0C1,

	CAND_MCU_MODULE_TEMP   	= 0x0A0,
	CAND_MCU_TORQUE_SPEED  	= 0x0A1,
	CAND_MCU_MODULE_PHASE  	= 0x0A2,
	CAND_MCU_CURRENT      	= 0x0A3,
	CAND_MCU_FAULT_CODE   	= 0x0A4,
	CAND_MCU_MODULATION    	= 0x0A5,
	CAND_MCU_STATES					= 0x0A6,

	CAND_MCU_ERROR_1				= 0x0A7,
	CAND_MCU_ERROR_2				= 0x0A8,
	CAND_MCU_ERROR_3				= 0x0AB,
	CAND_MCU_ERROR_4				= 0x0AC,
	CAND_MCU_ERROR_5				= 0x0AD,
	CAND_MCU_ERROR_6				= 0x0AF,
} CAN_ADDR;

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
