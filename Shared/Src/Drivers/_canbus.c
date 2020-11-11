/*
 * _can.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja Kusuma
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "can.h"

/* External variables ---------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;
#if (!BOOTLOADER)
extern osMutexId_t CanTxMutexHandle;
extern osMessageQueueId_t CanRxQueueHandle;
#endif

/* Exported variables ---------------------------------------------------------*/
uint8_t CAN_ACTIVE = 0;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void CANBUS_Header(CAN_TxHeaderTypeDef *TxHeader, uint32_t address, uint32_t DLC);
static uint8_t CANBUS_IsActivated(void);

/* Public functions implementation ---------------------------------------------*/
void CANBUS_Init(void) {
	uint8_t error = 0;

	HAL_GPIO_WritePin(INT_CAN_PWR_GPIO_Port, INT_CAN_PWR_Pin, 0);
	_DelayMS(100);
	HAL_GPIO_WritePin(INT_CAN_PWR_GPIO_Port, INT_CAN_PWR_Pin, 1);
	_DelayMS(500);

	MX_CAN1_Init();

	if (!CANBUS_Filter())
		error = 1;

	if (!error)
		if (HAL_CAN_Start(&hcan1) != HAL_OK)
			error = 1;

#if (!BOOTLOADER)
	if (!error)
		if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
			error = 1;

#endif

	if (error)
		Error_Handler();

	CAN_ACTIVE = !error;
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

	return (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) == HAL_OK);
}

/*----------------------------------------------------------------------------
 write a message to CAN peripheral and transmit it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Write(uint32_t address, CAN_DATA *TxData, uint32_t DLC) {
	CAN_TxHeaderTypeDef TxHeader;
	HAL_StatusTypeDef status;

	if (!CANBUS_IsActivated())
		return 0;

	lock();
	CANBUS_Header(&TxHeader, address, DLC);
	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
		;

	/* Start the Transmission process */
	status = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData->u8, NULL);

	// debugging
	if (status == HAL_OK)
		CANBUS_TxDebugger(&TxHeader, TxData);

	unlock();
	return (status == HAL_OK);
}

/*----------------------------------------------------------------------------
 read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Read(can_rx_t *Rx) {
	HAL_StatusTypeDef status = HAL_ERROR;

	if (!CANBUS_IsActivated())
		return 0;

	lock();
	/* Check FIFO */
	if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0)) {
		/* Get RX message */
		status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &(Rx->header), Rx->data.u8);
		// debugging
		if (status == HAL_OK)
			CANBUS_RxDebugger(Rx);

	}
	unlock();

	return (status == HAL_OK);
}

uint32_t CANBUS_ReadID(CAN_RxHeaderTypeDef *RxHeader) {
	if (RxHeader->IDE == CAN_ID_STD)
		return RxHeader->StdId;
	return RxHeader->ExtId;
}

void CANBUS_TxDebugger(CAN_TxHeaderTypeDef *TxHeader, CAN_DATA *TxData) {
	// debugging
	LOG_Str("\n[TX] ");
	if (TxHeader->IDE == CAN_ID_STD)
		LOG_Hex32(TxHeader->StdId);
	else
		LOG_Hex32(TxHeader->ExtId);

	LOG_Str(" => ");
	if (TxHeader->RTR == CAN_RTR_DATA)
		LOG_BufHex((char*) TxData, TxHeader->DLC);
	else
		LOG_Str("RTR");

	LOG_Enter();
}

void CANBUS_RxDebugger(can_rx_t *Rx) {
	// debugging
	LOG_Str("\n[RX] ");
	LOG_Hex32(CANBUS_ReadID(&(Rx->header)));
	LOG_Str(" <= ");
	if (Rx->header.RTR == CAN_RTR_DATA)
		LOG_BufHex(Rx->data.CHAR, Rx->header.DLC);
	else
		LOG_Str("RTR");

	LOG_Enter();
}

#if (!BOOTLOADER)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	can_rx_t Rx;
	// signal only when RTOS started
	if (CANBUS_Read(&Rx))
		if (osKernelGetState() == osKernelRunning)
			osMessageQueuePut(CanRxQueueHandle, &Rx, 0U, 0U);

}
#endif

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
#if (!BOOTLOADER)
	osMutexAcquire(CanTxMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (!BOOTLOADER)
	osMutexRelease(CanTxMutexHandle);
#endif
}

static void CANBUS_Header(CAN_TxHeaderTypeDef *TxHeader, uint32_t address, uint32_t DLC) {
	/* Configure Transmission process */
	if (address > 0x7FF) {
		TxHeader->IDE = CAN_ID_EXT;
		TxHeader->ExtId = address;
	} else {
		TxHeader->IDE = CAN_ID_STD;
		TxHeader->StdId = address;
	}
	TxHeader->RTR = (DLC ? CAN_RTR_DATA : CAN_RTR_REMOTE);
	TxHeader->DLC = DLC;
	TxHeader->TransmitGlobalTime = DISABLE;
}

static uint8_t CANBUS_IsActivated(void) {
	if (!CAN_ACTIVE) {
		CANBUS_Init();
		_DelayMS(1000);
	}
	return CAN_ACTIVE;
}
