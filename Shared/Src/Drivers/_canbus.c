/*
 * _can.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja Kusuma
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "can.h"
#if (RTOS_ENABLE)
#include "Nodes/NODE.h"
#endif

/* External variables
 * ----------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMessageQueueId_t CanRxQueueHandle;
extern osMutexId_t CanTxMutexHandle;
#endif

/* Private variables
 * ----------------------------------------------------------*/
static can_t can = {
		.pcan = &hcan1
};

/* Private functions declaration
 * ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void Reset(void);
static void Header(CAN_TxHeaderTypeDef *header, uint32_t address, uint32_t DLC, uint8_t ext);
#if CAN_DEBUG
static void TxDebugger(CAN_TxHeaderTypeDef *TxHeader, CAN_DATA *TxData);
static void RxDebugger(CAN_RxHeaderTypeDef *RxHeader, CAN_DATA *RxData);
#endif

/* Public functions implementation
 * ---------------------------------------------*/
void CANBUS_Init(void) {
	uint8_t e;

	GATE_CanbusReset();
	MX_CAN1_Init();

	e = (!CANBUS_Filter());

	if (!e)
		e = (HAL_CAN_Start(can.pcan) != HAL_OK);

#if (RTOS_ENABLE)
	if (!e)
		e = (HAL_CAN_ActivateNotification(can.pcan, CAN_IT_RX_FIFO0_MSG_PENDING) !=
				HAL_OK);
#endif

	if (e)
		printf("CAN: Initiate error.\n");
}

void CANBUS_DeInit(void) {
	HAL_CAN_DeactivateNotification(can.pcan, CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_CAN_Stop(can.pcan);
	HAL_CAN_DeInit(can.pcan);
	GATE_CanbusShutdown();
}

uint8_t CANBUS_Filter(void) {
	CAN_FilterTypeDef sFilterConfig;

	/* Configure the CAN Filter */
	sFilterConfig.FilterBank = 0;
	// set filter to mask mode (not id_list mode)
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	// set 32-bit scale configuration
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	// assign filter to FIFO 0
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	// activate filter
	sFilterConfig.FilterActivation = ENABLE;

	return (HAL_CAN_ConfigFilter(can.pcan, &sFilterConfig) == HAL_OK);
}

/*----------------------------------------------------------------------------
 write a message to CAN peripheral and transmit it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Write(can_tx_t *Tx, uint32_t address, uint32_t DLC, uint8_t ext) {
	HAL_StatusTypeDef status;
	uint32_t tick;

	lock();
	Header(&(Tx->header), address, DLC, ext);
	tick = _GetTickMS();
	while (_GetTickMS() - tick < CAN_RX_MS && HAL_CAN_GetTxMailboxesFreeLevel(can.pcan) == 0);

	/* Start the Transmission process */
	status = HAL_CAN_AddTxMessage(can.pcan, &(Tx->header), Tx->data.u8, NULL);

#if CAN_DEBUG
	if (status == HAL_OK)
		TxDebugger(&(Tx->header), &(Tx->data));
#endif
	if (status != HAL_OK)
		Reset();

	unlock();
	return (status == HAL_OK);
}

/*----------------------------------------------------------------------------
 read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Read(can_rx_t *Rx) {
	HAL_StatusTypeDef status = HAL_ERROR;

	memset(Rx, 0x00, sizeof(can_rx_t));

	lock();
	if (HAL_CAN_GetRxFifoFillLevel(can.pcan, CAN_RX_FIFO0)) {
		status = HAL_CAN_GetRxMessage(can.pcan, CAN_RX_FIFO0, &(Rx->header), Rx->data.u8);

#if CAN_DEBUG
		if (status == HAL_OK)
			RxDebugger(&(Rx->header), &(Rx->data));
#endif
		if (status != HAL_OK)
			Reset();
	}

	unlock();

	return (status == HAL_OK);
}

uint32_t CANBUS_ReadID(CAN_RxHeaderTypeDef *RxHeader) {
	if (RxHeader->IDE == CAN_ID_STD)
		return RxHeader->StdId;
	return RxHeader->ExtId;
}

#if (RTOS_ENABLE)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	can_rx_t Rx;

	if (CANBUS_Read(&Rx))
		if (osKernelGetState() == osKernelRunning)
			osMessageQueuePut(CanRxQueueHandle, &Rx, 0U, 0U);
}
#endif

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(CanTxMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
	osMutexRelease(CanTxMutexHandle);
#endif
}

static void Reset(void) {
	CANBUS_DeInit();
	CANBUS_Init();
}

static void Header(CAN_TxHeaderTypeDef *header, uint32_t address, uint32_t DLC, uint8_t ext) {
	if (ext) {
		header->IDE = CAN_ID_EXT;
		header->ExtId = address;
	} else {
		header->IDE = CAN_ID_STD;
		header->StdId = address;
	}
	header->DLC = DLC;
	header->RTR = (DLC ? CAN_RTR_DATA : CAN_RTR_REMOTE);
	header->TransmitGlobalTime = DISABLE;
}

#if CAN_DEBUG
static void TxDebugger(CAN_TxHeaderTypeDef *TxHeader, CAN_DATA *TxData) {
	printf("CAN:[TX] 0x%08X => %.*s\n",
			(unsigned int)((TxHeader->IDE == CAN_ID_STD) ? TxHeader->StdId
					: TxHeader->ExtId),
					  (TxHeader->RTR == CAN_RTR_DATA) ? (int)TxHeader->DLC : strlen("RTR"),
							  (TxHeader->RTR == CAN_RTR_DATA) ? TxData->CHAR : "RTR");
}

static void RxDebugger(CAN_RxHeaderTypeDef *RxHeader, CAN_DATA *RxData) {
	printf("CAN:[RX] 0x%08X <=  %.*s\n", (unsigned int)CANBUS_ReadID(RxHeader),
			(RxHeader->RTR == CAN_RTR_DATA) ? (int)RxHeader->DLC : strlen("RTR"),
					(RxHeader->RTR == CAN_RTR_DATA) ? RxData->CHAR : "RTR");
}
#endif
