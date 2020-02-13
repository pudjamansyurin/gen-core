/*
 * _can.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja Kusuma
 */

#include "_can.h"

extern osThreadId CanRxTaskHandle;
extern osMutexId CanTxMutexHandle;
extern CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef *CanHandle = &hcan1;
CAN_Rx RxCan;

void CAN_Init(void) {
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

	/* Configure the CAN Filter */
	if (HAL_CAN_ConfigFilter(CanHandle, &sFilterConfig) != HAL_OK) {
		/* Start Error */
		Error_Handler();
	}

	/* Start the CAN peripheral */
	if (HAL_CAN_Start(CanHandle) != HAL_OK) {
		/* Start Error */
		Error_Handler();
	}

	/* Activate CAN RX notification */
	if (HAL_CAN_ActivateNotification(CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
		/* Notification Error */
		Error_Handler();
	}
}

void CAN_Set_Tx_Header(CAN_TxHeaderTypeDef *TxHeader, uint32_t StdId, uint32_t DLC) {
	/* Configure Global Transmission process */
	TxHeader->RTR = CAN_RTR_DATA;
	TxHeader->IDE = CAN_ID_STD;
	TxHeader->TransmitGlobalTime = DISABLE;
	TxHeader->StdId = StdId;
	TxHeader->DLC = DLC;
}

HAL_StatusTypeDef CAN_Filter(void) {
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

	return HAL_CAN_ConfigFilter(CanHandle, &sFilterConfig);
}

/*----------------------------------------------------------------------------
 wite a message to CAN peripheral and transmit it
 *----------------------------------------------------------------------------*/
uint8_t CAN_Write(CAN_Tx *TxCan) {
	osMutexWait(CanTxMutexHandle, osWaitForever);

	uint32_t TxMailbox;
	HAL_StatusTypeDef status;
	// check tx mailbox is ready
	while (HAL_CAN_GetTxMailboxesFreeLevel(CanHandle) == 0)
		;
	/* Start the Transmission process */
	status = HAL_CAN_AddTxMessage(CanHandle, &(TxCan->TxHeader), (uint8_t*) &(TxCan->TxData), &TxMailbox);

	// debugging
//	if (status == HAL_OK) {
//		SWV_SendStr("\n[TX] ");
//		SWV_SendHex32(TxCan->TxHeader.StdId);
//		SWV_SendStr(" => ");
//		SWV_SendBufHex((char*) &(TxCan->TxData), sizeof(TxCan->TxData));
//		SWV_SendStrLn("");
//	}

	osMutexRelease(CanTxMutexHandle);
	return (status == HAL_OK);
}

/*----------------------------------------------------------------------------
 read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
uint8_t CAN_Read(CAN_Rx *RxCan) {
	HAL_StatusTypeDef status;

	/* Get RX message */
	status = HAL_CAN_GetRxMessage(CanHandle, CAN_RX_FIFO0, &(RxCan->RxHeader), RxCan->RxData);

	// debugging
	if (status == HAL_OK) {
		SWV_SendStr("\n[RX] ");
		SWV_SendHex32(RxCan->RxHeader.StdId);
		SWV_SendStr(" <= ");
		SWV_SendBufHex((char*) &(RxCan->RxData), sizeof(RxCan->RxData));
		SWV_SendStrLn("");
	}

	return (status == HAL_OK);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// read rx fifo
	if (CAN_Read(&RxCan)) {
		// signal only when RTOS started
		if (osKernelRunning()) {
			xTaskNotifyFromISR(CanRxTaskHandle, EVENT_CAN_RX_IT, eSetBits, &xHigherPriorityTaskWoken);
		}
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
