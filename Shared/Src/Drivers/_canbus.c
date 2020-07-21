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

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void CANBUS_Header(uint32_t address, CAN_TxHeaderTypeDef *TxHeader, uint32_t DLC);

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
uint8_t CANBUS_Write(uint32_t address, CAN_DATA *TxData, uint32_t DLC) {
    CAN_TxHeaderTypeDef TxHeader;
    HAL_StatusTypeDef status;

    lock();
    // set header
    CANBUS_Header(address, &TxHeader, DLC);

    /* Wait transmission complete */
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
        ;

    /* Start the Transmission process */
    status = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData->u8, NULL);

    // debugging
    //    if (status == HAL_OK) {
    //        CANBUS_TxDebugger();
    //    }

    unlock();
    return (status == HAL_OK);
}

/*----------------------------------------------------------------------------
 read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
uint8_t CANBUS_Read(can_rx_t *Rx) {
    HAL_StatusTypeDef status = HAL_ERROR;

    lock();
    /* Check FIFO */
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0)) {
        /* Get RX message */
        status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &(Rx->header), Rx->data.u8);
        // debugging
        //        if (status == HAL_OK) {
        //            CANBUS_RxDebugger();
        //        }
    }
    unlock();

    return (status == HAL_OK);
}

uint32_t CANBUS_ReadID(CAN_RxHeaderTypeDef *RxHeader) {
    if (RxHeader->IDE == CAN_ID_STD) {
        return RxHeader->StdId;
    }
    return RxHeader->ExtId;
}

void CANBUS_TxDebugger(can_tx_t *Tx) {
    // debugging
    LOG_Str("\n[TX] ");
    if (Tx->header.IDE == CAN_ID_STD) {
        LOG_Hex32(Tx->header.StdId);
    } else {
        LOG_Hex32(Tx->header.ExtId);
    }
    LOG_Str(" => ");
    if (Tx->header.RTR == CAN_RTR_DATA) {
        LOG_BufHex((char*) &(Tx->data), sizeof(Tx->data));
    } else {
        LOG_Str("RTR");
    }
    LOG_Enter();
}

void CANBUS_RxDebugger(can_rx_t *Rx) {
    // debugging
    LOG_Str("\n[RX] ");
    LOG_Hex32(CANBUS_ReadID(&(Rx->header)));
    LOG_Str(" <= ");
    if (Rx->header.RTR == CAN_RTR_DATA) {
        LOG_BufHex(Rx->data.CHAR, sizeof(Rx->data.CHAR));
    } else {
        LOG_Str("RTR");
    }
    LOG_Enter();
}

#if (!BOOTLOADER)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    // signal only when RTOS started
    if (osKernelGetState() == osKernelRunning) {
        osThreadFlagsSet(CanRxTaskHandle, EVT_CAN_RX_IT);
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

static void CANBUS_Header(uint32_t address, CAN_TxHeaderTypeDef *TxHeader, uint32_t DLC) {
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
