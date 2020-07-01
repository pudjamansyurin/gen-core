/*
 * _can.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Puja Kusuma
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"

/* External variables ---------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;
#if (!BOOTLOADER)
extern osThreadId_t CanRxTaskHandle;
extern osMutexId_t CanTxMutexHandle;
#endif

/* Public variables -----------------------------------------------------------*/
canbus_t CB;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void CANBUS_Header(uint32_t StdId, uint32_t DLC, uint8_t RTR);

/* Public functions implementation ---------------------------------------------*/
void CANBUS_Init(void) {
    /* Configure the CAN Filter */
    if (!CANBUS_Filter()) {
        /* Start Error */
        Error_Handler();
    }

    /* Start the CAN peripheral */
    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        /* Start Error */
        Error_Handler();
    }

#if (!BOOTLOADER)
    /* Activate CAN RX notification */
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        /* Notification Error */
        Error_Handler();
    }
#endif
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
 wite a message to CAN peripheral and transmit it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Write(uint32_t StdId, uint32_t DLC, uint8_t RTR) {
    canbus_tx_t *tx = &(CB.tx);
    uint32_t TxMailbox;
    HAL_StatusTypeDef status;

    lock();
    // check tx mailbox is ready
    while (!HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)) {
        _DelayMS(1);
    };

    // set header
    CANBUS_Header(StdId, DLC, RTR);

    /* Start the Transmission process */
    status = HAL_CAN_AddTxMessage(&hcan1, &(tx->header), tx->data.u8, &TxMailbox);

    // debugging
    if (status == HAL_OK) {
        CANBUS_TxDebugger();
    }

    unlock();
    return (status == HAL_OK);
}

/*----------------------------------------------------------------------------
 read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Read(void) {
    canbus_rx_t *rx = &(CB.rx);
    HAL_StatusTypeDef status;

    lock();
    /* Get RX message */
    status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &(rx->header), rx->data.u8);

    // debugging
    if (status == HAL_OK) {
        CANBUS_RxDebugger();
    }
    unlock();

    return (status == HAL_OK);
}

uint16_t CANBUS_ReadID(void) {
    if (CB.rx.header.IDE == CAN_ID_STD) {
        return CB.rx.header.StdId;
    }
    return _R(CB.rx.header.ExtId, 20);
}

void CANBUS_TxDebugger(void) {
    // debugging
    LOG_Str("\n[TX] ");
    if (CB.tx.header.IDE == CAN_ID_STD) {
        LOG_Hex32(CB.tx.header.StdId);
    } else {
        LOG_Hex32(CB.tx.header.ExtId);
    }
    LOG_Str(" => ");
    if (CB.tx.header.RTR == CAN_RTR_DATA) {
        LOG_BufHex((char*) &(CB.tx.data), sizeof(CB.tx.data));
    } else {
        LOG_Str("RTR");
    }
    LOG_Enter();
}

void CANBUS_RxDebugger(void) {
    // debugging
    LOG_Str("\n[RX] ");
    LOG_Hex32(CANBUS_ReadID());
    LOG_Str(" <= ");
    if (CB.rx.header.RTR == CAN_RTR_DATA) {
        LOG_BufHex(CB.rx.data.CHAR, sizeof(CB.rx.data.CHAR));
    } else {
        LOG_Str("RTR");
    }
    LOG_Enter();
}

#if (!BOOTLOADER)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    // read rx fifo
    if (CANBUS_Read()) {
        // signal only when RTOS started
        if (osKernelGetState() == osKernelRunning) {
            osThreadFlagsSet(CanRxTaskHandle, EVT_CAN_RX_IT);
        }
    }
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

static void CANBUS_Header(uint32_t StdId, uint32_t DLC, uint8_t RTR) {
    CAN_TxHeaderTypeDef *TxHeader = &(CB.tx.header);
    /* Configure Global Transmission process */
    TxHeader->RTR = (RTR ? CAN_RTR_REMOTE : CAN_RTR_DATA);
    TxHeader->IDE = CAN_ID_STD;
    TxHeader->TransmitGlobalTime = DISABLE;
    TxHeader->StdId = StdId;
    TxHeader->DLC = DLC;
}
