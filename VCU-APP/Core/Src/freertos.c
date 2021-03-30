/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include "aes.h"
#include "can.h"
#include "i2c.h"
#include "i2s.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

#include "DMA/_dma_finger.h"
#include "DMA/_dma_ublox.h"

#include "Drivers/_aes.h"
#include "Drivers/_bat.h"
#include "Drivers/_canbus.h"
#include "Drivers/_iwdg.h"
#include "Drivers/_rtc.h"
#include "Drivers/_simcom.h"

#include "Libs/_audio.h"
#include "Libs/_command.h"
#include "Libs/_eeprom.h"
#include "Libs/_finger.h"
#include "Libs/_firmware.h"
#include "Libs/_gps.h"
#include "Libs/_mems.h"
#include "Libs/_hbar.h"
#include "Libs/_mqtt.h"
#include "Libs/_remote.h"
#include "Libs/_reporter.h"

#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"
#include "Nodes/NODE.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticEventGroup_t osStaticEventGroupDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */
/* Definitions for ManagerTask */
osThreadId_t ManagerTaskHandle;
uint32_t ManagerTaskBuffer[ 256 ];
osStaticThreadDef_t ManagerTaskControlBlock;
const osThreadAttr_t ManagerTask_attributes = {
		.name = "ManagerTask",
		.cb_mem = &ManagerTaskControlBlock,
		.cb_size = sizeof(ManagerTaskControlBlock),
		.stack_mem = &ManagerTaskBuffer[0],
		.stack_size = sizeof(ManagerTaskBuffer),
		.priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for NetworkTask */
osThreadId_t NetworkTaskHandle;
uint32_t NetworkTaskBuffer[ 896 ];
osStaticThreadDef_t NetworkTaskControlBlock;
const osThreadAttr_t NetworkTask_attributes = {
		.name = "NetworkTask",
		.cb_mem = &NetworkTaskControlBlock,
		.cb_size = sizeof(NetworkTaskControlBlock),
		.stack_mem = &NetworkTaskBuffer[0],
		.stack_size = sizeof(NetworkTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ReporterTask */
osThreadId_t ReporterTaskHandle;
uint32_t ReporterTaskBuffer[ 304 ];
osStaticThreadDef_t ReporterTaskControlBlock;
const osThreadAttr_t ReporterTask_attributes = {
		.name = "ReporterTask",
		.cb_mem = &ReporterTaskControlBlock,
		.cb_size = sizeof(ReporterTaskControlBlock),
		.stack_mem = &ReporterTaskBuffer[0],
		.stack_size = sizeof(ReporterTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CommandTask */
osThreadId_t CommandTaskHandle;
uint32_t CommandTaskBuffer[ 328 ];
osStaticThreadDef_t CommandTaskControlBlock;
const osThreadAttr_t CommandTask_attributes = {
		.name = "CommandTask",
		.cb_mem = &CommandTaskControlBlock,
		.cb_size = sizeof(CommandTaskControlBlock),
		.stack_mem = &CommandTaskBuffer[0],
		.stack_size = sizeof(CommandTaskBuffer),
		.priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for GpsTask */
osThreadId_t GpsTaskHandle;
uint32_t GpsTaskBuffer[ 256 ];
osStaticThreadDef_t GpsTaskControlBlock;
const osThreadAttr_t GpsTask_attributes = {
		.name = "GpsTask",
		.cb_mem = &GpsTaskControlBlock,
		.cb_size = sizeof(GpsTaskControlBlock),
		.stack_mem = &GpsTaskBuffer[0],
		.stack_size = sizeof(GpsTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MemsTask */
osThreadId_t MemsTaskHandle;
uint32_t MemsTaskBuffer[ 304 ];
osStaticThreadDef_t MemsTaskControlBlock;
const osThreadAttr_t MemsTask_attributes = {
		.name = "MemsTask",
		.cb_mem = &MemsTaskControlBlock,
		.cb_size = sizeof(MemsTaskControlBlock),
		.stack_mem = &MemsTaskBuffer[0],
		.stack_size = sizeof(MemsTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RemoteTask */
osThreadId_t RemoteTaskHandle;
uint32_t RemoteTaskBuffer[ 256 ];
osStaticThreadDef_t RemoteTaskControlBlock;
const osThreadAttr_t RemoteTask_attributes = {
		.name = "RemoteTask",
		.cb_mem = &RemoteTaskControlBlock,
		.cb_size = sizeof(RemoteTaskControlBlock),
		.stack_mem = &RemoteTaskBuffer[0],
		.stack_size = sizeof(RemoteTaskBuffer),
		.priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for FingerTask */
osThreadId_t FingerTaskHandle;
uint32_t FingerTaskBuffer[ 224 ];
osStaticThreadDef_t FingerTaskControlBlock;
const osThreadAttr_t FingerTask_attributes = {
		.name = "FingerTask",
		.cb_mem = &FingerTaskControlBlock,
		.cb_size = sizeof(FingerTaskControlBlock),
		.stack_mem = &FingerTaskBuffer[0],
		.stack_size = sizeof(FingerTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for AudioTask */
osThreadId_t AudioTaskHandle;
uint32_t AudioTaskBuffer[ 240 ];
osStaticThreadDef_t AudioTaskControlBlock;
const osThreadAttr_t AudioTask_attributes = {
		.name = "AudioTask",
		.cb_mem = &AudioTaskControlBlock,
		.cb_size = sizeof(AudioTaskControlBlock),
		.stack_mem = &AudioTaskBuffer[0],
		.stack_size = sizeof(AudioTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CanRxTask */
osThreadId_t CanRxTaskHandle;
uint32_t CanRxTaskBuffer[ 229 ];
osStaticThreadDef_t CanRxTaskControlBlock;
const osThreadAttr_t CanRxTask_attributes = {
		.name = "CanRxTask",
		.cb_mem = &CanRxTaskControlBlock,
		.cb_size = sizeof(CanRxTaskControlBlock),
		.stack_mem = &CanRxTaskBuffer[0],
		.stack_size = sizeof(CanRxTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CanTxTask */
osThreadId_t CanTxTaskHandle;
uint32_t CanTxTaskBuffer[ 230 ];
osStaticThreadDef_t CanTxTaskControlBlock;
const osThreadAttr_t CanTxTask_attributes = {
		.name = "CanTxTask",
		.cb_mem = &CanTxTaskControlBlock,
		.cb_size = sizeof(CanTxTaskControlBlock),
		.stack_mem = &CanTxTaskBuffer[0],
		.stack_size = sizeof(CanTxTaskBuffer),
		.priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Hmi2PowerTask */
osThreadId_t Hmi2PowerTaskHandle;
uint32_t Hmi2PowerTaskBuffer[ 176 ];
osStaticThreadDef_t Hmi2PowerTaskControlBlock;
const osThreadAttr_t Hmi2PowerTask_attributes = {
		.name = "Hmi2PowerTask",
		.cb_mem = &Hmi2PowerTaskControlBlock,
		.cb_size = sizeof(Hmi2PowerTaskControlBlock),
		.stack_mem = &Hmi2PowerTaskBuffer[0],
		.stack_size = sizeof(Hmi2PowerTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GateTask */
osThreadId_t GateTaskHandle;
uint32_t GateTaskBuffer[ 224 ];
osStaticThreadDef_t GateTaskControlBlock;
const osThreadAttr_t GateTask_attributes = {
		.name = "GateTask",
		.cb_mem = &GateTaskControlBlock,
		.cb_size = sizeof(GateTaskControlBlock),
		.stack_mem = &GateTaskBuffer[0],
		.stack_size = sizeof(GateTaskBuffer),
		.priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for CommandQueue */
osMessageQueueId_t CommandQueueHandle;
uint8_t CommandQueueBuffer[ 1 * sizeof( command_t ) ];
osStaticMessageQDef_t CommandQueueControlBlock;
const osMessageQueueAttr_t CommandQueue_attributes = {
		.name = "CommandQueue",
		.cb_mem = &CommandQueueControlBlock,
		.cb_size = sizeof(CommandQueueControlBlock),
		.mq_mem = &CommandQueueBuffer,
		.mq_size = sizeof(CommandQueueBuffer)
};
/* Definitions for ResponseQueue */
osMessageQueueId_t ResponseQueueHandle;
uint8_t ResponseQueueBuffer[ 1 * sizeof( response_t ) ];
osStaticMessageQDef_t ResponseQueueControlBlock;
const osMessageQueueAttr_t ResponseQueue_attributes = {
		.name = "ResponseQueue",
		.cb_mem = &ResponseQueueControlBlock,
		.cb_size = sizeof(ResponseQueueControlBlock),
		.mq_mem = &ResponseQueueBuffer,
		.mq_size = sizeof(ResponseQueueBuffer)
};
/* Definitions for ReportQueue */
osMessageQueueId_t ReportQueueHandle;
uint8_t ReportQueueBuffer[ 100 * sizeof( report_t ) ];
osStaticMessageQDef_t ReportQueueControlBlock;
const osMessageQueueAttr_t ReportQueue_attributes = {
		.name = "ReportQueue",
		.cb_mem = &ReportQueueControlBlock,
		.cb_size = sizeof(ReportQueueControlBlock),
		.mq_mem = &ReportQueueBuffer,
		.mq_size = sizeof(ReportQueueBuffer)
};
/* Definitions for DriverQueue */
osMessageQueueId_t DriverQueueHandle;
uint8_t DriverQueueBuffer[ 1 * sizeof( uint8_t ) ];
osStaticMessageQDef_t DriverQueueControlBlock;
const osMessageQueueAttr_t DriverQueue_attributes = {
		.name = "DriverQueue",
		.cb_mem = &DriverQueueControlBlock,
		.cb_size = sizeof(DriverQueueControlBlock),
		.mq_mem = &DriverQueueBuffer,
		.mq_size = sizeof(DriverQueueBuffer)
};
/* Definitions for CanRxQueue */
osMessageQueueId_t CanRxQueueHandle;
uint8_t CanRxQueueBuffer[ 10 * sizeof( can_rx_t ) ];
osStaticMessageQDef_t CanRxQueueControlBlock;
const osMessageQueueAttr_t CanRxQueue_attributes = {
		.name = "CanRxQueue",
		.cb_mem = &CanRxQueueControlBlock,
		.cb_size = sizeof(CanRxQueueControlBlock),
		.mq_mem = &CanRxQueueBuffer,
		.mq_size = sizeof(CanRxQueueBuffer)
};
/* Definitions for QuotaQueue */
osMessageQueueId_t QuotaQueueHandle;
uint8_t QuotaQueueBuffer[ 1 * 200 ];
osStaticMessageQDef_t QuotaQueueControlBlock;
const osMessageQueueAttr_t QuotaQueue_attributes = {
		.name = "QuotaQueue",
		.cb_mem = &QuotaQueueControlBlock,
		.cb_size = sizeof(QuotaQueueControlBlock),
		.mq_mem = &QuotaQueueBuffer,
		.mq_size = sizeof(QuotaQueueBuffer)
};
/* Definitions for UssdQueue */
osMessageQueueId_t UssdQueueHandle;
uint8_t UssdQueueBuffer[ 1 * 20 ];
osStaticMessageQDef_t UssdQueueControlBlock;
const osMessageQueueAttr_t UssdQueue_attributes = {
		.name = "UssdQueue",
		.cb_mem = &UssdQueueControlBlock,
		.cb_size = sizeof(UssdQueueControlBlock),
		.mq_mem = &UssdQueueBuffer,
		.mq_size = sizeof(UssdQueueBuffer)
};
/* Definitions for EepromMutex */
osMutexId_t EepromMutexHandle;
osStaticMutexDef_t EepromMutexControlBlock;
const osMutexAttr_t EepromMutex_attributes = {
		.name = "EepromMutex",
		.cb_mem = &EepromMutexControlBlock,
		.cb_size = sizeof(EepromMutexControlBlock),
};
/* Definitions for RtcMutex */
osMutexId_t RtcMutexHandle;
osStaticMutexDef_t RtcMutexControlBlock;
const osMutexAttr_t RtcMutex_attributes = {
		.name = "RtcMutex",
		.cb_mem = &RtcMutexControlBlock,
		.cb_size = sizeof(RtcMutexControlBlock),
};
/* Definitions for CrcMutex */
osMutexId_t CrcMutexHandle;
osStaticMutexDef_t CrcMutexControlBlock;
const osMutexAttr_t CrcMutex_attributes = {
		.name = "CrcMutex",
		.cb_mem = &CrcMutexControlBlock,
		.cb_size = sizeof(CrcMutexControlBlock),
};
/* Definitions for AesMutex */
osMutexId_t AesMutexHandle;
osStaticMutexDef_t AesMutexControlBlock;
const osMutexAttr_t AesMutex_attributes = {
		.name = "AesMutex",
		.cb_mem = &AesMutexControlBlock,
		.cb_size = sizeof(AesMutexControlBlock),
};
/* Definitions for RngMutex */
osMutexId_t RngMutexHandle;
osStaticMutexDef_t RngMutexControlBlock;
const osMutexAttr_t RngMutex_attributes = {
		.name = "RngMutex",
		.cb_mem = &RngMutexControlBlock,
		.cb_size = sizeof(RngMutexControlBlock),
};
/* Definitions for BatMutex */
osMutexId_t BatMutexHandle;
osStaticMutexDef_t BatMutexControlBlock;
const osMutexAttr_t BatMutex_attributes = {
		.name = "BatMutex",
		.cb_mem = &BatMutexControlBlock,
		.cb_size = sizeof(BatMutexControlBlock),
};
/* Definitions for CanTxMutex */
osMutexId_t CanTxMutexHandle;
osStaticMutexDef_t CanTxMutexControlBlock;
const osMutexAttr_t CanTxMutex_attributes = {
		.name = "CanTxMutex",
		.cb_mem = &CanTxMutexControlBlock,
		.cb_size = sizeof(CanTxMutexControlBlock),
};
/* Definitions for IwdgMutex */
osMutexId_t IwdgMutexHandle;
osStaticMutexDef_t IwdgMutexControlBlock;
const osMutexAttr_t IwdgMutex_attributes = {
		.name = "IwdgMutex",
		.cb_mem = &IwdgMutexControlBlock,
		.cb_size = sizeof(IwdgMutexControlBlock),
};
/* Definitions for BuzzerMutex */
osMutexId_t BuzzerMutexHandle;
osStaticMutexDef_t BuzzerMutexControlBlock;
const osMutexAttr_t BuzzerMutex_attributes = {
		.name = "BuzzerMutex",
		.cb_mem = &BuzzerMutexControlBlock,
		.cb_size = sizeof(BuzzerMutexControlBlock),
};
/* Definitions for LogRecMutex */
osMutexId_t LogRecMutexHandle;
osStaticMutexDef_t LogRecMutexControlBlock;
const osMutexAttr_t LogRecMutex_attributes = {
		.name = "LogRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &LogRecMutexControlBlock,
		.cb_size = sizeof(LogRecMutexControlBlock),
};
/* Definitions for SimcomRecMutex */
osMutexId_t SimcomRecMutexHandle;
osStaticMutexDef_t SimcomRecMutexControlBlock;
const osMutexAttr_t SimcomRecMutex_attributes = {
		.name = "SimcomRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &SimcomRecMutexControlBlock,
		.cb_size = sizeof(SimcomRecMutexControlBlock),
};
/* Definitions for RemoteRecMutex */
osMutexId_t RemoteRecMutexHandle;
osStaticMutexDef_t RemoteRecMutexControlBlock;
const osMutexAttr_t RemoteRecMutex_attributes = {
		.name = "RemoteRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &RemoteRecMutexControlBlock,
		.cb_size = sizeof(RemoteRecMutexControlBlock),
};
/* Definitions for FingerRecMutex */
osMutexId_t FingerRecMutexHandle;
osStaticMutexDef_t FingerRecMutexControlBlock;
const osMutexAttr_t FingerRecMutex_attributes = {
		.name = "FingerRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &FingerRecMutexControlBlock,
		.cb_size = sizeof(FingerRecMutexControlBlock),
};
/* Definitions for GpsRecMutex */
osMutexId_t GpsRecMutexHandle;
osStaticMutexDef_t GpsRecMutexControlBlock;
const osMutexAttr_t GpsRecMutex_attributes = {
		.name = "GpsRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &GpsRecMutexControlBlock,
		.cb_size = sizeof(GpsRecMutexControlBlock),
};
/* Definitions for MemsRecMutex */
osMutexId_t MemsRecMutexHandle;
osStaticMutexDef_t MemsRecMutexControlBlock;
const osMutexAttr_t MemsRecMutex_attributes = {
		.name = "MemsRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &MemsRecMutexControlBlock,
		.cb_size = sizeof(MemsRecMutexControlBlock),
};
/* Definitions for AudioRecMutex */
osMutexId_t AudioRecMutexHandle;
osStaticMutexDef_t AudioRecMutexControlBlock;
const osMutexAttr_t AudioRecMutex_attributes = {
		.name = "AudioRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &AudioRecMutexControlBlock,
		.cb_size = sizeof(AudioRecMutexControlBlock),
};
/* Definitions for GlobalEvent */
osEventFlagsId_t GlobalEventHandle;
osStaticEventGroupDef_t GlobalEventControlBlock;
const osEventFlagsAttr_t GlobalEvent_attributes = {
		.name = "GlobalEvent",
		.cb_mem = &GlobalEventControlBlock,
		.cb_size = sizeof(GlobalEventControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartManagerTask(void *argument);
void StartNetworkTask(void *argument);
void StartReporterTask(void *argument);
void StartCommandTask(void *argument);
void StartGpsTask(void *argument);
void StartMemsTask(void *argument);
void StartRemoteTask(void *argument);
void StartFingerTask(void *argument);
void StartAudioTask(void *argument);
void StartCanRxTask(void *argument);
void StartCanTxTask(void *argument);
void StartHmi2PowerTask(void *argument);
void StartGateTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(TaskHandle_t xTask,
		signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
called if a stack overflow is detected. */
	printf("%s is overflowed.\n", pcTaskName);

	while (1) {
		IWDG_Refresh();
		GATE_LedToggle();
		HAL_Delay(100);
	}
}
/* USER CODE END 4 */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */
	/* Create the mutex(es) */
	/* creation of EepromMutex */
	EepromMutexHandle = osMutexNew(&EepromMutex_attributes);

	/* creation of RtcMutex */
	RtcMutexHandle = osMutexNew(&RtcMutex_attributes);

	/* creation of CrcMutex */
	CrcMutexHandle = osMutexNew(&CrcMutex_attributes);

	/* creation of AesMutex */
	AesMutexHandle = osMutexNew(&AesMutex_attributes);

	/* creation of RngMutex */
	RngMutexHandle = osMutexNew(&RngMutex_attributes);

	/* creation of BatMutex */
	BatMutexHandle = osMutexNew(&BatMutex_attributes);

	/* creation of CanTxMutex */
	CanTxMutexHandle = osMutexNew(&CanTxMutex_attributes);

	/* creation of IwdgMutex */
	IwdgMutexHandle = osMutexNew(&IwdgMutex_attributes);

	/* creation of BuzzerMutex */
	BuzzerMutexHandle = osMutexNew(&BuzzerMutex_attributes);

	/* Create the recursive mutex(es) */
	/* creation of LogRecMutex */
	LogRecMutexHandle = osMutexNew(&LogRecMutex_attributes);

	/* creation of SimcomRecMutex */
	SimcomRecMutexHandle = osMutexNew(&SimcomRecMutex_attributes);

	/* creation of RemoteRecMutex */
	RemoteRecMutexHandle = osMutexNew(&RemoteRecMutex_attributes);

	/* creation of FingerRecMutex */
	FingerRecMutexHandle = osMutexNew(&FingerRecMutex_attributes);

	/* creation of GpsRecMutex */
	GpsRecMutexHandle = osMutexNew(&GpsRecMutex_attributes);

	/* creation of MemsRecMutex */
	MemsRecMutexHandle = osMutexNew(&MemsRecMutex_attributes);

	/* creation of AudioRecMutex */
	AudioRecMutexHandle = osMutexNew(&AudioRecMutex_attributes);

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of CommandQueue */
	CommandQueueHandle = osMessageQueueNew (1, sizeof(command_t), &CommandQueue_attributes);

	/* creation of ResponseQueue */
	ResponseQueueHandle = osMessageQueueNew (1, sizeof(response_t), &ResponseQueue_attributes);

	/* creation of ReportQueue */
	ReportQueueHandle = osMessageQueueNew (100, sizeof(report_t), &ReportQueue_attributes);

	/* creation of DriverQueue */
	DriverQueueHandle = osMessageQueueNew (1, sizeof(uint8_t), &DriverQueue_attributes);

	/* creation of CanRxQueue */
	CanRxQueueHandle = osMessageQueueNew (10, sizeof(can_rx_t), &CanRxQueue_attributes);

	/* creation of QuotaQueue */
	QuotaQueueHandle = osMessageQueueNew (1, 200, &QuotaQueue_attributes);

	/* creation of UssdQueue */
	UssdQueueHandle = osMessageQueueNew (1, 20, &UssdQueue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of ManagerTask */
	ManagerTaskHandle = osThreadNew(StartManagerTask, NULL, &ManagerTask_attributes);

	/* creation of NetworkTask */
	NetworkTaskHandle = osThreadNew(StartNetworkTask, NULL, &NetworkTask_attributes);

	/* creation of ReporterTask */
	ReporterTaskHandle = osThreadNew(StartReporterTask, NULL, &ReporterTask_attributes);

	/* creation of CommandTask */
	CommandTaskHandle = osThreadNew(StartCommandTask, NULL, &CommandTask_attributes);

	/* creation of GpsTask */
	GpsTaskHandle = osThreadNew(StartGpsTask, NULL, &GpsTask_attributes);

	/* creation of MemsTask */
	MemsTaskHandle = osThreadNew(StartMemsTask, NULL, &MemsTask_attributes);

	/* creation of RemoteTask */
	RemoteTaskHandle = osThreadNew(StartRemoteTask, NULL, &RemoteTask_attributes);

	/* creation of FingerTask */
	FingerTaskHandle = osThreadNew(StartFingerTask, NULL, &FingerTask_attributes);

	/* creation of AudioTask */
	AudioTaskHandle = osThreadNew(StartAudioTask, NULL, &AudioTask_attributes);

	/* creation of CanRxTask */
	CanRxTaskHandle = osThreadNew(StartCanRxTask, NULL, &CanRxTask_attributes);

	/* creation of CanTxTask */
	CanTxTaskHandle = osThreadNew(StartCanTxTask, NULL, &CanTxTask_attributes);

	/* creation of Hmi2PowerTask */
	Hmi2PowerTaskHandle = osThreadNew(StartHmi2PowerTask, NULL, &Hmi2PowerTask_attributes);

	/* creation of GateTask */
	GateTaskHandle = osThreadNew(StartGateTask, NULL, &GateTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* creation of GlobalEvent */
	GlobalEventHandle = osEventFlagsNew(&GlobalEvent_attributes);

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartManagerTask */
/**
 * @brief  Function implementing the ManagerTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartManagerTask */
void StartManagerTask(void *argument)
{
	/* USER CODE BEGIN StartManagerTask */
	TickType_t lastWake;
	float mps;

	// Initiate, this task get executed first!
	VCU.Init();
	NODE.Init();

	// Peripheral Initiate
	BAT_Init();
	EEPROM_Init();
	RTC_Init();

	// Threads management:
	//  osThreadSuspend(NetworkTaskHandle);
	//  osThreadSuspend(ReporterTaskHandle);
	//  osThreadSuspend(CommandTaskHandle);
	//  osThreadSuspend(GpsTaskHandle);
	//  osThreadSuspend(MemsTaskHandle);
	//  osThreadSuspend(RemoteTaskHandle);
	//  osThreadSuspend(FingerTaskHandle);
	osThreadSuspend(AudioTaskHandle);
	//  osThreadSuspend(CanRxTaskHandle);
	//  osThreadSuspend(CanTxTaskHandle);
	//  osThreadSuspend(GateTaskHandle);
	osThreadSuspend(Hmi2PowerTaskHandle);

	// Check thread creation
	if (!_osCheckRTOS())
		return;

	// Release threads
	osEventFlagsSet(GlobalEventHandle, EVENT_READY);

	/* Infinite loop */
	for (;;) {
		TASKS.tick.manager = _GetTickMS();
		lastWake = _GetTickMS();

		_osCheckTasks();

		VCU.Refresh();
		VCU.CheckState();
		NODE.Refresh();

		mps = (float) MCU.RpmToSpeed(MCU.d.rpm) / 3.6;
		VCU.SetOdometer(mps * MANAGER_WAKEUP / 1000);

		IWDG_Refresh();
		osDelayUntil(lastWake + MANAGER_WAKEUP);
	}
	/* USER CODE END StartManagerTask */
}

/* USER CODE BEGIN Header_StartNetworkTask */
/**
 * @brief Function implementing the NetworkTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartNetworkTask */
void StartNetworkTask(void *argument)
{
	/* USER CODE BEGIN StartNetworkTask */
	uint32_t notif;
	command_t cmd;
	report_t report;
	response_t response;
	payload_t payloads[] = {
			{
					.type = PAYLOAD_RESPONSE,
					.pQueue = &ResponseQueueHandle,
					.pPayload = &response,
					.pending = 0
			}, {
					.type = PAYLOAD_REPORT,
					.pQueue = &ReportQueueHandle,
					.pPayload = &report,
					.pending = 0
			}
	};

	_osEventManager();

	// Initiate
	Simcom_Init();
	Simcom_SetState(SIM_STATE_SERVER_ON, 0);

	/* Infinite loop */
	for (;;) {
		TASKS.tick.network = _GetTickMS();

		if (_osFlagAny(&notif, 100)) {
			if (notif & FLAG_NET_REPORT_DISCARD)
				payloads[PAYLOAD_REPORT].pending = 0;

			if (notif & FLAG_NET_READ_SMS || notif & FLAG_NET_SEND_USSD) {
				char buf[200] = {0};
				uint8_t ok = 0;

				if (notif & FLAG_NET_SEND_USSD) {
					char ussd[20];
					if (osMessageQueueGet(UssdQueueHandle, ussd, NULL, 0U) == osOK)
						ok = Simcom_SendUSSD(ussd, buf, sizeof(buf));
				} else if (notif & FLAG_NET_READ_SMS)
					ok = Simcom_ReadNewSMS(buf, sizeof(buf));

				if (ok)
					ok = _osQueuePutRst(QuotaQueueHandle, buf);

				osThreadFlagsSet(CommandTaskHandle,	ok ? FLAG_COMMAND_OK : FLAG_COMMAND_ERROR);
			}
		}

		// SIMCOM related routines
		if (RTC_NeedCalibration())
			Simcom_CalibrateTime();

		// Upload Payloads (Report & Response)
		for (uint8_t i=0; i<PAYLOAD_MAX; i++)
			if (RPT_PayloadPending(&payloads[i]))
				if (RPT_WrapPayload(&payloads[i]))
					if (Simcom_SetState(SIM_STATE_MQTT_ON, 0))
						if (MQTT_Publish(&payloads[i]))
							payloads[i].pending = 0;

		// Check Command
		if (Simcom_SetState(SIM_STATE_MQTT_ON, 0)) {
			if (MQTT_GotCommand()) {
				MQTT_AckPublish(&cmd);
				CMD_ExecuteCommand(&cmd);
			}
		}
	}
	/* USER CODE END StartNetworkTask */
}

/* USER CODE BEGIN Header_StartReporterTask */
/**
 * @brief Function implementing the ReporterTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReporterTask */
void StartReporterTask(void *argument)
{
	/* USER CODE BEGIN StartReporterTask */
	uint32_t notif;
	uint16_t interval;
	report_t report;
	FRAME_TYPE frame;

	_osEventManager();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.reporter = _GetTickMS();

		frame = RPT_FrameDecider();
		interval = RPT_IntervalDecider();

		RPT_ReportCapture(frame, &report);

		// Put report to log
		while(_osQueuePut(ReportQueueHandle, &report) == 0) {
			osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_REPORT_DISCARD);
			_DelayMS(1);
		}

		// reset some events group
		VCU.SetEvent(EVG_NET_SOFT_RESET, 0);
		VCU.SetEvent(EVG_NET_HARD_RESET, 0);

		_osFlagOne(&notif, FLAG_REPORTER_YIELD, interval * 1000);
	}
	/* USER CODE END StartReporterTask */
}

/* USER CODE BEGIN Header_StartCommandTask */
/**
 * @brief Function implementing the CommandTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommandTask */
void StartCommandTask(void *argument)
{
	/* USER CODE BEGIN StartCommandTask */
	command_t cmd;
	response_t resp;

	_osEventManager();

	// Handle Post-FOTA
	if (FW_PostFota(&resp))
		_osQueuePut(ResponseQueueHandle, &resp);

	/* Infinite loop */
	for (;;) {
		TASKS.tick.command = _GetTickMS();
		// get command in queue
		if (osMessageQueueGet(CommandQueueHandle, &cmd, NULL, osWaitForever) ==	osOK) {
			uint8_t val = *(uint8_t *)cmd.data.value;
			uint8_t code = cmd.header.code;
			uint8_t sub_code = cmd.header.sub_code;

			// default command response
			uint8_t *res_code = &(resp.data.res_code);
			resp.header.code = code;
			resp.header.sub_code = sub_code;
			resp.data.res_code = RESPONSE_STATUS_OK;
			strcpy(resp.data.message, "");

			// handle the command
			if (code == CMD_CODE_GEN) {
				switch (sub_code) {
				case CMD_GEN_INFO:
					CMD_GenInfo(&resp);
					break;

				case CMD_GEN_LED:
					GATE_LedWrite(val);
					break;

				case CMD_GEN_RTC:
					RTC_Write(*(datetime_t *)cmd.data.value);
					break;

				case CMD_GEN_ODOM:
					EEPROM_Odometer(EE_CMD_W, (*(uint32_t *)cmd.data.value) * 1000);
					break;

				default:
					*res_code = RESPONSE_STATUS_INVALID;
					break;
				}
			}

			else if (code == CMD_CODE_OVERRIDE) {
				switch (sub_code) {

				case CMD_OVERRIDE_STATE:
					if (VCU.d.state < VEHICLE_NORMAL) {
						sprintf(resp.data.message, "State should >= {%d}.", VEHICLE_NORMAL);
						*res_code = RESPONSE_STATUS_ERROR;
					} else
						VCU.d.override.state = val;
					break;

				case CMD_OVERRIDE_RPT_INTERVAL:
					RPT.override.interval = *(uint16_t *)cmd.data.value;
					break;

				case CMD_OVERRIDE_RPT_FRAME:
					RPT.override.frame = val;
					break;

				case CMD_OVERRIDE_RMT_SEAT:
					RMT_OpenSeat();
					break;

				case CMD_OVERRIDE_RMT_ALARM:
					RMT_BeepAlarm();
					break;

				default:
					*res_code = RESPONSE_STATUS_INVALID;
					break;
				}
			}

			else if (code == CMD_CODE_AUDIO) {
				if (VCU.d.state < VEHICLE_NORMAL) {
					sprintf(resp.data.message, "State should >= {%d}.", VEHICLE_NORMAL);
					*res_code = RESPONSE_STATUS_ERROR;
				} else
					switch (sub_code) {
					case CMD_AUDIO_BEEP:
						osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_BEEP);
						break;

					case CMD_AUDIO_MUTE:
						osThreadFlagsSet(AudioTaskHandle, val ? FLAG_AUDIO_MUTE_ON : FLAG_AUDIO_MUTE_OFF);
						break;

					default:
						*res_code = RESPONSE_STATUS_INVALID;
						break;
					}
			}

			else if (code == CMD_CODE_FINGER) {
				if (VCU.d.state < VEHICLE_STANDBY) {
					sprintf(resp.data.message, "State should >= {%d}.", VEHICLE_STANDBY);
					*res_code = RESPONSE_STATUS_ERROR;
				} else
					switch (sub_code) {
					case CMD_FINGER_FETCH:
						osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_FETCH);
						CMD_FingerFetch(&resp);
						break;

					case CMD_FINGER_ADD:
						osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_ADD);
						CMD_FingerAdd(&resp, DriverQueueHandle);
						break;

					case CMD_FINGER_DEL:
						_osQueuePutRst(DriverQueueHandle, &val);
						osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_DEL);
						CMD_Finger(&resp);
						break;

					case CMD_FINGER_RST:
						osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_RST);
						CMD_Finger(&resp);
						break;

					default:
						*res_code = RESPONSE_STATUS_INVALID;
						break;
					}
			}

			else if (code == CMD_CODE_REMOTE) {
				if (VCU.d.state < VEHICLE_NORMAL) {
					sprintf(resp.data.message, "State should >= {%d}.", VEHICLE_NORMAL);
					*res_code = RESPONSE_STATUS_ERROR;
				} else
					switch (sub_code) {
					case CMD_REMOTE_PAIRING:
						osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_PAIRING);
						CMD_RemotePairing(&resp);
						break;

					default:
						*res_code = RESPONSE_STATUS_INVALID;
						break;
					}
			}

			else if (code == CMD_CODE_FOTA) {
				*res_code = RESPONSE_STATUS_ERROR;

				if (VCU.d.state == VEHICLE_RUN) {
					sprintf(resp.data.message, "State should != {%d}.", VEHICLE_RUN);
				} else
					switch (sub_code) {
					case CMD_FOTA_VCU:
						FW_EnterModeIAP(IAP_VCU, resp.data.message);
						break;

					case CMD_FOTA_HMI:
						FW_EnterModeIAP(IAP_HMI, resp.data.message);
						break;

					default:
						*res_code = RESPONSE_STATUS_INVALID;
						break;
					}
			}

			else if (code == CMD_CODE_NET) {
				*res_code = RESPONSE_STATUS_ERROR;

				switch (sub_code) {
				case CMD_NET_SEND_USSD:
					_osQueuePutRst(UssdQueueHandle, cmd.data.value);
					osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_SEND_USSD);
					CMD_NetQuota(&resp, QuotaQueueHandle);
					break;

				case CMD_NET_READ_SMS:
					osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_READ_SMS);
					CMD_NetQuota(&resp, QuotaQueueHandle);
					break;

				default:
					*res_code = RESPONSE_STATUS_INVALID;
					break;
				}
			}

			else if (code == CMD_CODE_HBAR) {
				switch (sub_code) {

				case CMD_HBAR_DRIVE:
					HBAR.d.mode[HBAR_M_DRIVE] = val;
					break;

				case CMD_HBAR_TRIP:
					HBAR.d.mode[HBAR_M_TRIP] = val;
					break;

				case CMD_HBAR_REPORT:
					HBAR.d.mode[HBAR_M_REPORT] = val;
					break;

				case CMD_HBAR_REVERSE:
					HBAR.state[HBAR_K_REVERSE] = val;
					break;

				default:
					*res_code = RESPONSE_STATUS_INVALID;
					break;
				}
			}

			else if (code == CMD_CODE_MCU) {
				if (!MCU.d.active) {
					sprintf(resp.data.message, "MCU not active!");
					*res_code = RESPONSE_STATUS_ERROR;
				} else
					switch (sub_code) {

					case CMD_MCU_SPEED_MAX:
						MCU.SetSpeedMax(val);
						break;

					case CMD_MCU_TEMPLATES:
						MCU.SetTemplates(val);
						break;

					default:
						*res_code = RESPONSE_STATUS_INVALID;
						break;
					}
			}

			else
				*res_code = RESPONSE_STATUS_INVALID;

			// Get current snapshot
			RPT_ResponseCapture(&resp);
			_osQueuePutRst(ResponseQueueHandle, &resp);
		}
	}
	/* USER CODE END StartCommandTask */
}

/* USER CODE BEGIN Header_StartGpsTask */
/**
 * @brief Function implementing the GpsTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGpsTask */
void StartGpsTask(void *argument)
{
	/* USER CODE BEGIN StartGpsTask */
	uint32_t notif;

	_osEventManager();

	GPS_Init();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.gps = _GetTickMS();

		// Check notifications
		if (_osFlagOne(&notif, FLAG_GPS_RECEIVED, 1000)) {
			// nmea ready, do something

			//		if ((meter = GPS_CalculateOdometer()))
			//			VCU.SetOdometer(meter);
		}

		HBAR_AccumulateTrip(255);
		GPS_Refresh();
	}
	/* USER CODE END StartGpsTask */
}

/* USER CODE BEGIN Header_StartMemsTask */
/**
 * @brief Function implementing the MemsTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMemsTask */
void StartMemsTask(void *argument)
{
	/* USER CODE BEGIN StartMemsTask */
	uint32_t notif;
	uint8_t fallen;

	_osEventManager();
	_osFlagOne(&notif, FLAG_MEMS_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	MEMS_Init();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.mems = _GetTickMS();

		// Check notifications
		if (_osFlagAny(&notif, 1000)) {
			if (notif & FLAG_MEMS_TASK_STOP) {
				VCU.SetEvent(EVG_BIKE_FALLEN, 0);
				VCU.SetEvent(EVG_BIKE_MOVED, 0);

				MEMS_DeInit();
				_osFlagOne(&notif, FLAG_MEMS_TASK_START, osWaitForever);
				MEMS_Init();
			}

			if (notif & FLAG_MEMS_DETECTOR_RESET)
				MEMS_ResetDetector();
		}

		// Read all data
		if (MEMS_Capture()) {
			fallen = MEMS_Process();
			VCU.SetEvent(EVG_BIKE_FALLEN, fallen);
			osThreadFlagsSet(AudioTaskHandle, fallen ? FLAG_AUDIO_BEEP_START : FLAG_AUDIO_BEEP_STOP);

			// Drag detector
			if (VCU.d.state < VEHICLE_STANDBY) {
				if (MEMS.drag.init) {
					MEMS_ActivateDetector();
				} else if (MEMS_Dragged()) {
					GATE_HornToggle(250);
					GATE_HornToggle(250);
					VCU.SetEvent(EVG_BIKE_MOVED, 1);
				}
			}
			else MEMS_ResetDetector();
		}

		MEMS_Refresh();
	}
	/* USER CODE END StartMemsTask */
}

/* USER CODE BEGIN Header_StartRemoteTask */
/**
 * @brief Function implementing the RemoteTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartRemoteTask */
void StartRemoteTask(void *argument)
{
	/* USER CODE BEGIN StartRemoteTask */
	uint32_t notif;
	RMT_CMD command;

	_osEventManager();
	_osFlagOne(&notif, FLAG_REMOTE_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	AES_Init();
	RMT_Init();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.remote = _GetTickMS();

		RMT_Refresh(VCU.d.state);

		if (_osFlagAny(&notif, 2)) {
			if (notif & FLAG_REMOTE_TASK_STOP) {
				VCU.SetEvent(EVG_REMOTE_MISSING, 1);

				RMT_DeInit();
				_osFlagOne(&notif, FLAG_REMOTE_TASK_START, osWaitForever);
				RMT_Init();
			}

			if (notif & FLAG_REMOTE_PAIRING)
				RMT_Pairing();

			if (notif & FLAG_REMOTE_RX_IT) {
				if (RMT_ValidateCommand(&command)) {
					if (command == RMT_CMD_PING) {
						if (RMT_GotPairedResponse())
							osThreadFlagsSet(CommandTaskHandle, FLAG_COMMAND_OK);
					}
					else if (command == RMT_CMD_ALARM)
						RMT_BeepAlarm();
					else if (command == RMT_CMD_SEAT)
						RMT_OpenSeat();
				}
			}
		}

	}
	/* USER CODE END StartRemoteTask */
}

/* USER CODE BEGIN Header_StartFingerTask */
/**
 * @brief Function implementing the FingerTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartFingerTask */
void StartFingerTask(void *argument)
{
	/* USER CODE BEGIN StartFingerTask */
	uint32_t notif;
	uint8_t id;

	_osEventManager();
	_osFlagOne(&notif, FLAG_FINGER_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	FINGER_Init();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.finger = _GetTickMS();

		if (_osFlagAny(&notif, 1000)) {
			if (notif & FLAG_FINGER_TASK_STOP) {
				FINGER_DeInit();
				_osFlagOne(&notif, FLAG_FINGER_TASK_START, osWaitForever);
				FINGER_Init();
			}

			if (notif & FLAG_FINGER_PLACED) {
				id = FINGER_Auth();
				if (id > 0) {
					FGR.d.id = FGR.d.id ? 0: id;
					GATE_LedBlink(200);	_DelayMS(100);
					GATE_LedBlink(200);
				} else
					GATE_LedBlink(1000);
			}

			else {
				uint8_t ok = 0;

				if (notif & FLAG_FINGER_ADD) {
					if (FINGER_Enroll(&id, &ok))
						_osQueuePutRst(DriverQueueHandle, &id);
				}
				if (notif & FLAG_FINGER_DEL) {
					if (osMessageQueueGet(DriverQueueHandle, &id, NULL, 0U) == osOK)
						ok = FINGER_DeleteID(id);
				}
				if (notif & FLAG_FINGER_FETCH) {
					ok = FINGER_Fetch();
				}
				if (notif & FLAG_FINGER_RST)
					ok = FINGER_ResetDB();

				osThreadFlagsSet(CommandTaskHandle, ok ? FLAG_COMMAND_OK : FLAG_COMMAND_ERROR);
			}
		}
	}
	/* USER CODE END StartFingerTask */
}

/* USER CODE BEGIN Header_StartAudioTask */
/**
 * @brief Function implementing the AudioTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartAudioTask */
void StartAudioTask(void *argument)
{
	/* USER CODE BEGIN StartAudioTask */
	uint32_t notif;

	_osEventManager();
	_osFlagOne(&notif, FLAG_AUDIO_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	/* Initiate Wave player (Codec, DMA, I2C) */
	AUDIO_Init();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.audio = _GetTickMS();

		if (_osFlagAny(&notif, 1000)) {
			if (notif & FLAG_AUDIO_TASK_STOP) {
				AUDIO_DeInit();
				_osFlagOne(&notif, FLAG_AUDIO_TASK_START, osWaitForever);
				AUDIO_Init();
			}

			// Beep command
			if (notif & FLAG_AUDIO_BEEP) {
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
				_DelayMS(250);
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
			}

			// Long-Beep Command
			if (notif & FLAG_AUDIO_BEEP_START)
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 0);
			if (notif & FLAG_AUDIO_BEEP_STOP)
				AUDIO_BeepStop();

			// Mute command
			if (notif & FLAG_AUDIO_MUTE_ON)
				AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
			if (notif & FLAG_AUDIO_MUTE_OFF)
				AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
		}

		if (AUDIO.d.volume != MCU.SpeedToVolume())
			AUDIO_OUT_SetVolume(MCU.SpeedToVolume());
		AUDIO_Refresh();
	}
	/* USER CODE END StartAudioTask */
}

/* USER CODE BEGIN Header_StartCanRxTask */
/**
 * @brief Function implementing the CanRxTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanRxTask */
void StartCanRxTask(void *argument)
{
	/* USER CODE BEGIN StartCanRxTask */
	uint32_t notif;
	can_rx_t Rx;

	_osEventManager();
	_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	/* Infinite loop */
	for (;;) {
		TASKS.tick.canRx = _GetTickMS();

		// Check notifications
		if (_osFlagOne(&notif, FLAG_CAN_TASK_STOP, 0)) {
			NODE.Init();
			_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
		}

		if (osMessageQueueGet(CanRxQueueHandle, &Rx, NULL, 1000) == osOK) {
			if (Rx.header.IDE == CAN_ID_STD) {
				switch (Rx.header.StdId) {
				case CAND_HMI1:
					HMI1.r.State(&Rx);
					break;
				case CAND_HMI2:
					HMI2.r.State(&Rx);
					break;
				case CAND_MCU_CURRENT_DC:
					MCU.r.CurrentDC(&Rx);
					break;
				case CAND_MCU_VOLTAGE_DC:
					MCU.r.VoltageDC(&Rx);
					break;
				case CAND_MCU_TORQUE_SPEED:
					MCU.r.TorqueSpeed(&Rx);
					break;
				case CAND_MCU_FAULT_CODE:
					MCU.r.FaultCode(&Rx);
					break;
				case CAND_MCU_STATE:
					MCU.r.State(&Rx);
					break;
				case CAND_MCU_TEMPLATE_R:
					MCU.r.Template(&Rx);
					break;
				default:
					break;
				}
			} else {
				switch (BMS_CAND(Rx.header.ExtId)) {
				case BMS_CAND(CAND_BMS_PARAM_1):
																							BMS.r.Param1(&Rx);
				break;
				case BMS_CAND(CAND_BMS_PARAM_2):
																							BMS.r.Param2(&Rx);
				break;
				default:
					break;
				}
			}
		}
	}
	/* USER CODE END StartCanRxTask */
}

/* USER CODE BEGIN Header_StartCanTxTask */
/**
 * @brief Function implementing the CanTxTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanTxTask */
void StartCanTxTask(void *argument)
{
	/* USER CODE BEGIN StartCanTxTask */
	uint32_t notif;
	TickType_t last500ms, last1000ms;

	_osEventManager();
	_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	// initiate
	CANBUS_Init();

	/* Infinite loop */
	last500ms = _GetTickMS();
	last1000ms = _GetTickMS();
	for (;;) {
		TASKS.tick.canTx = _GetTickMS();

		// Check notifications
		if (_osFlagAny(&notif, 20)) {
			if (notif & FLAG_CAN_TASK_STOP) {
				HMI2.PowerByCan(0);
				MCU.PowerOverCan(0);
				BMS.PowerOverCan(0);

				CANBUS_DeInit();
				_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
				CANBUS_Init();
			}
		}

		// send every 20ms
		if (HMI1.d.active)
			VCU.t.SwitchControl();

		// send every 500ms
		if (_GetTickMS() - last500ms > 500) {
			last500ms = _GetTickMS();

			if (HMI1.d.active)
				VCU.t.MixedData();
		}

		// send every 1000ms
		if (_GetTickMS() - last1000ms > 1000) {
			last1000ms = _GetTickMS();

			if (HMI1.d.active) {
				VCU.t.Datetime(RTC_Read());
				VCU.t.TripData();
			}

			VCU.t.Heartbeat();

			HMI2.PowerByCan(VCU.d.state >= VEHICLE_STANDBY);
			MCU.PowerOverCan(BMS.d.run && VCU.d.state == VEHICLE_RUN);
			BMS.PowerOverCan(VCU.d.state == VEHICLE_RUN);
		}
	}
	/* USER CODE END StartCanTxTask */
}

/* USER CODE BEGIN Header_StartHmi2PowerTask */
/**
 * @brief Function implementing the Hmi2PowerTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartHmi2PowerTask */
void StartHmi2PowerTask(void *argument)
{
	/* USER CODE BEGIN StartHmi2PowerTask */
	uint32_t notif;

	_osEventManager();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.hmi2Power = _GetTickMS();

		if (_osFlagOne(&notif, FLAG_HMI2POWER_CHANGED, osWaitForever)) {

			if (HMI2.d.powerRequest)
				while (!HMI2.d.run)
					HMI2.PowerOn();

			else
				while (HMI2.d.run)
					HMI2.PowerOff();
		}
	}
	/* USER CODE END StartHmi2PowerTask */
}

/* USER CODE BEGIN Header_StartGateTask */
/**
 * @brief Function implementing the GateTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGateTask */
void StartGateTask(void *argument)
{
	/* USER CODE BEGIN StartGateTask */
	uint32_t notif;

	_osEventManager();

	// Initiate
	HBAR_Init();
	HBAR_ReadStates();

	/* Infinite loop */
	for (;;) {
		TASKS.tick.gate = _GetTickMS();

		// wait forever
		if (_osFlagOne(&notif, FLAG_GATE_HBAR, 10)) {
			// handle bounce effect
			_DelayMS(50);

			HBAR_ReadStarter();
			if (VCU.d.state >= VEHICLE_STANDBY)
				HBAR_ReadStates();

			osThreadFlagsClear(FLAG_GATE_HBAR);
		}

		HBAR_RefreshSelectSet();
		HMI1.Power(VCU.d.state >= VEHICLE_STANDBY);
		GATE_System12v(VCU.d.state >= VEHICLE_STANDBY);
	}
	/* USER CODE END StartGateTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (osKernelGetState() != osKernelRunning)
		return;

	if (osEventFlagsGet(GlobalEventHandle) != EVENT_READY)
		return;

	if (GPIO_Pin == INT_REMOTE_IRQ_Pin)
		if (VCU.d.state >= VEHICLE_NORMAL)
			RMT_IrqHandler();

	if (GPIO_Pin == EXT_FINGER_IRQ_Pin)
		if (VCU.d.state >= VEHICLE_STANDBY)
			osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_PLACED);

	if (GPIO_Pin != INT_REMOTE_IRQ_Pin && GPIO_Pin != EXT_FINGER_IRQ_Pin)
		if (VCU.d.state >= VEHICLE_NORMAL)
			osThreadFlagsSet(GateTaskHandle, FLAG_GATE_HBAR);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
