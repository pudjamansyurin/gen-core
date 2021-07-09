/*
 * _can.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_can.h"

#include "can.h"

#if (APP)
#include "Nodes/NODE.h"
#endif

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMessageQueueId_t CanRxQueueHandle;
extern osMutexId_t CanTxMutexHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define CAN_RX_MS ((uint16_t)1000)

/* Private variables
 * --------------------------------------------*/
CAN_HandleTypeDef* pcan = &hcan1;

/* Private functions prototype
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);
static void Reset(void);
static uint8_t Filter(void);
static void Header(CAN_TxHeaderTypeDef* header, uint32_t address, uint32_t DLC,
                   uint8_t ext);
#if CAN_DEBUG
static void TxDebug(CAN_TxHeaderTypeDef* TxHeader, CAN_DATA* TxData);
static void RxDebug(CAN_RxHeaderTypeDef* RxHeader, CAN_DATA* RxData);
#endif

/* Public functions implementation
 * --------------------------------------------*/
void CAN_Init(void) {
  uint8_t e;

  GATE_CanbusReset();
  MX_CAN1_Init();

  e = !Filter();

  if (!e) e = (HAL_CAN_Start(pcan) != HAL_OK);

#if (APP)
  if (!e)
    e = (HAL_CAN_ActivateNotification(pcan, CAN_IT_RX_FIFO0_MSG_PENDING) !=
         HAL_OK);
#endif

  if (e) printf("CAN: Initiate error.\n");
}

void CAN_DeInit(void) {
  HAL_CAN_DeactivateNotification(pcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_Stop(pcan);
  HAL_CAN_DeInit(pcan);
  GATE_CanbusShutdown();
}

uint8_t CAN_Write(can_tx_t* Tx, uint32_t address, uint32_t DLC, uint8_t ext) {
  HAL_StatusTypeDef status;
  uint32_t tick;

  Lock();
  Header(&(Tx->header), address, DLC, ext);
  tick = tickMs();
  while (tickIn(tick, CAN_RX_MS) &&
         HAL_CAN_GetTxMailboxesFreeLevel(pcan) == 0) {
  };

  /* Start the Transmission process */
  status = HAL_CAN_AddTxMessage(pcan, &(Tx->header), Tx->data.u8, NULL);

#if CAN_DEBUG
  if (status == HAL_OK) TxDebug(&(Tx->header), &(Tx->data));
#endif
  if (status != HAL_OK) Reset();

  UnLock();
  return (status == HAL_OK);
}

uint8_t CAN_Read(can_rx_t* Rx) {
  HAL_StatusTypeDef status = HAL_ERROR;

  memset(Rx, 0x00, sizeof(can_rx_t));

  Lock();
  if (HAL_CAN_GetRxFifoFillLevel(pcan, CAN_RX_FIFO0)) {
    status =
        HAL_CAN_GetRxMessage(pcan, CAN_RX_FIFO0, &(Rx->header), Rx->data.u8);

#if CAN_DEBUG
    if (status == HAL_OK) RxDebug(&(Rx->header), &(Rx->data));
#endif
    if (status != HAL_OK) Reset();
  }

  UnLock();

  return (status == HAL_OK);
}

uint32_t CAN_ReadID(CAN_RxHeaderTypeDef* RxHeader) {
  if (RxHeader->IDE == CAN_ID_STD) return RxHeader->StdId;
  return RxHeader->ExtId;
}

#if (APP)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
  can_rx_t Rx;

  if (CAN_Read(&Rx))
    if (osKernelGetState() == osKernelRunning)
      osMessageQueuePut(CanRxQueueHandle, &Rx, 0U, 0U);
}
#endif

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(CanTxMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(CanTxMutexHandle);
#endif
}

static void Reset(void) {
  CAN_DeInit();
  CAN_Init();
}

static uint8_t Filter(void) {
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

  return (HAL_CAN_ConfigFilter(pcan, &sFilterConfig) == HAL_OK);
}

static void Header(CAN_TxHeaderTypeDef* header, uint32_t address, uint32_t DLC,
                   uint8_t ext) {
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
static void TxDebug(CAN_TxHeaderTypeDef* TxHeader, CAN_DATA* TxData) {
  printf("CAN:[TX] 0x%08X => %.*s\n",
         (unsigned int)((TxHeader->IDE == CAN_ID_STD) ? TxHeader->StdId
                                                      : TxHeader->ExtId),
         (TxHeader->RTR == CAN_RTR_DATA) ? (int)TxHeader->DLC : strlen("RTR"),
         (TxHeader->RTR == CAN_RTR_DATA) ? TxData->CHAR : "RTR");
}

static void RxDebug(CAN_RxHeaderTypeDef* RxHeader, CAN_DATA* RxData) {
  printf("CAN:[RX] 0x%08X <=  %.*s\n", (unsigned int)CAN_ReadID(RxHeader),
         (RxHeader->RTR == CAN_RTR_DATA) ? (int)RxHeader->DLC : strlen("RTR"),
         (RxHeader->RTR == CAN_RTR_DATA) ? RxData->CHAR : "RTR");
}
#endif
