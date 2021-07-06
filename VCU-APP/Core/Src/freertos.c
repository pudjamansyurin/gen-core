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

#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "App/_command.h"
#include "App/_exec.h"
#include "App/_firmware.h"
#include "App/_ml.h"
#include "App/_network.h"
#include "App/_reporter.h"
#include "App/_state.h"
#include "App/_task.h"
#include "DMA/_dma_finger.h"
#include "DMA/_dma_ublox.h"
#include "Drivers/_aes.h"
#include "Drivers/_bat.h"
#include "Drivers/_canbus.h"
#include "Drivers/_iwdg.h"
#include "Drivers/_simcom.h"
#include "Libs/_audio.h"
#include "Libs/_eeprom.h"
#include "Libs/_finger.h"
#include "Libs/_gps.h"
#include "Libs/_hbar.h"
#include "Libs/_mems.h"
#include "Libs/_mqtt.h"
#include "Libs/_remote.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/MCU.h"
#include "Nodes/NODE.h"
#include "Nodes/VCU.h"
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
uint32_t ManagerTaskBuffer[328];
osStaticThreadDef_t ManagerTaskControlBlock;
const osThreadAttr_t ManagerTask_attributes = {
		.name = "ManagerTask",
		.cb_mem = &ManagerTaskControlBlock,
		.cb_size = sizeof(ManagerTaskControlBlock),
		.stack_mem = &ManagerTaskBuffer[0],
		.stack_size = sizeof(ManagerTaskBuffer),
		.priority = (osPriority_t)osPriorityRealtime,
};
/* Definitions for NetworkTask */
osThreadId_t NetworkTaskHandle;
uint32_t NetworkTaskBuffer[896];
osStaticThreadDef_t NetworkTaskControlBlock;
const osThreadAttr_t NetworkTask_attributes = {
		.name = "NetworkTask",
		.cb_mem = &NetworkTaskControlBlock,
		.cb_size = sizeof(NetworkTaskControlBlock),
		.stack_mem = &NetworkTaskBuffer[0],
		.stack_size = sizeof(NetworkTaskBuffer),
		.priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for ReporterTask */
osThreadId_t ReporterTaskHandle;
uint32_t ReporterTaskBuffer[304];
osStaticThreadDef_t ReporterTaskControlBlock;
const osThreadAttr_t ReporterTask_attributes = {
		.name = "ReporterTask",
		.cb_mem = &ReporterTaskControlBlock,
		.cb_size = sizeof(ReporterTaskControlBlock),
		.stack_mem = &ReporterTaskBuffer[0],
		.stack_size = sizeof(ReporterTaskBuffer),
		.priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for CommandTask */
osThreadId_t CommandTaskHandle;
uint32_t CommandTaskBuffer[328];
osStaticThreadDef_t CommandTaskControlBlock;
const osThreadAttr_t CommandTask_attributes = {
		.name = "CommandTask",
		.cb_mem = &CommandTaskControlBlock,
		.cb_size = sizeof(CommandTaskControlBlock),
		.stack_mem = &CommandTaskBuffer[0],
		.stack_size = sizeof(CommandTaskBuffer),
		.priority = (osPriority_t)osPriorityAboveNormal,
};
/* Definitions for MemsTask */
osThreadId_t MemsTaskHandle;
uint32_t MemsTaskBuffer[304];
osStaticThreadDef_t MemsTaskControlBlock;
const osThreadAttr_t MemsTask_attributes = {
		.name = "MemsTask",
		.cb_mem = &MemsTaskControlBlock,
		.cb_size = sizeof(MemsTaskControlBlock),
		.stack_mem = &MemsTaskBuffer[0],
		.stack_size = sizeof(MemsTaskBuffer),
		.priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for RemoteTask */
osThreadId_t RemoteTaskHandle;
uint32_t RemoteTaskBuffer[256];
osStaticThreadDef_t RemoteTaskControlBlock;
const osThreadAttr_t RemoteTask_attributes = {
		.name = "RemoteTask",
		.cb_mem = &RemoteTaskControlBlock,
		.cb_size = sizeof(RemoteTaskControlBlock),
		.stack_mem = &RemoteTaskBuffer[0],
		.stack_size = sizeof(RemoteTaskBuffer),
		.priority = (osPriority_t)osPriorityHigh,
};
/* Definitions for FingerTask */
osThreadId_t FingerTaskHandle;
uint32_t FingerTaskBuffer[328];
osStaticThreadDef_t FingerTaskControlBlock;
const osThreadAttr_t FingerTask_attributes = {
		.name = "FingerTask",
		.cb_mem = &FingerTaskControlBlock,
		.cb_size = sizeof(FingerTaskControlBlock),
		.stack_mem = &FingerTaskBuffer[0],
		.stack_size = sizeof(FingerTaskBuffer),
		.priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for AudioTask */
osThreadId_t AudioTaskHandle;
uint32_t AudioTaskBuffer[240];
osStaticThreadDef_t AudioTaskControlBlock;
const osThreadAttr_t AudioTask_attributes = {
		.name = "AudioTask",
		.cb_mem = &AudioTaskControlBlock,
		.cb_size = sizeof(AudioTaskControlBlock),
		.stack_mem = &AudioTaskBuffer[0],
		.stack_size = sizeof(AudioTaskBuffer),
		.priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for CanRxTask */
osThreadId_t CanRxTaskHandle;
uint32_t CanRxTaskBuffer[229];
osStaticThreadDef_t CanRxTaskControlBlock;
const osThreadAttr_t CanRxTask_attributes = {
		.name = "CanRxTask",
		.cb_mem = &CanRxTaskControlBlock,
		.cb_size = sizeof(CanRxTaskControlBlock),
		.stack_mem = &CanRxTaskBuffer[0],
		.stack_size = sizeof(CanRxTaskBuffer),
		.priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for CanTxTask */
osThreadId_t CanTxTaskHandle;
uint32_t CanTxTaskBuffer[230];
osStaticThreadDef_t CanTxTaskControlBlock;
const osThreadAttr_t CanTxTask_attributes = {
		.name = "CanTxTask",
		.cb_mem = &CanTxTaskControlBlock,
		.cb_size = sizeof(CanTxTaskControlBlock),
		.stack_mem = &CanTxTaskBuffer[0],
		.stack_size = sizeof(CanTxTaskBuffer),
		.priority = (osPriority_t)osPriorityAboveNormal,
};
/* Definitions for GateTask */
osThreadId_t GateTaskHandle;
uint32_t GateTaskBuffer[224];
osStaticThreadDef_t GateTaskControlBlock;
const osThreadAttr_t GateTask_attributes = {
		.name = "GateTask",
		.cb_mem = &GateTaskControlBlock,
		.cb_size = sizeof(GateTaskControlBlock),
		.stack_mem = &GateTaskBuffer[0],
		.stack_size = sizeof(GateTaskBuffer),
		.priority = (osPriority_t)osPriorityAboveNormal,
};
/* Definitions for CommandQueue */
osMessageQueueId_t CommandQueueHandle;
uint8_t CommandQueueBuffer[1 * sizeof(command_t)];
osStaticMessageQDef_t CommandQueueControlBlock;
const osMessageQueueAttr_t CommandQueue_attributes = {
		.name = "CommandQueue",
		.cb_mem = &CommandQueueControlBlock,
		.cb_size = sizeof(CommandQueueControlBlock),
		.mq_mem = &CommandQueueBuffer,
		.mq_size = sizeof(CommandQueueBuffer)};
/* Definitions for ResponseQueue */
osMessageQueueId_t ResponseQueueHandle;
uint8_t ResponseQueueBuffer[1 * sizeof(response_t)];
osStaticMessageQDef_t ResponseQueueControlBlock;
const osMessageQueueAttr_t ResponseQueue_attributes = {
		.name = "ResponseQueue",
		.cb_mem = &ResponseQueueControlBlock,
		.cb_size = sizeof(ResponseQueueControlBlock),
		.mq_mem = &ResponseQueueBuffer,
		.mq_size = sizeof(ResponseQueueBuffer)};
/* Definitions for ReportQueue */
osMessageQueueId_t ReportQueueHandle;
uint8_t ReportQueueBuffer[10 * sizeof(report_t)];
osStaticMessageQDef_t ReportQueueControlBlock;
const osMessageQueueAttr_t ReportQueue_attributes = {
		.name = "ReportQueue",
		.cb_mem = &ReportQueueControlBlock,
		.cb_size = sizeof(ReportQueueControlBlock),
		.mq_mem = &ReportQueueBuffer,
		.mq_size = sizeof(ReportQueueBuffer)};
/* Definitions for DriverQueue */
osMessageQueueId_t DriverQueueHandle;
uint8_t DriverQueueBuffer[1 * sizeof(uint8_t)];
osStaticMessageQDef_t DriverQueueControlBlock;
const osMessageQueueAttr_t DriverQueue_attributes = {
		.name = "DriverQueue",
		.cb_mem = &DriverQueueControlBlock,
		.cb_size = sizeof(DriverQueueControlBlock),
		.mq_mem = &DriverQueueBuffer,
		.mq_size = sizeof(DriverQueueBuffer)};
/* Definitions for CanRxQueue */
osMessageQueueId_t CanRxQueueHandle;
uint8_t CanRxQueueBuffer[10 * sizeof(can_rx_t)];
osStaticMessageQDef_t CanRxQueueControlBlock;
const osMessageQueueAttr_t CanRxQueue_attributes = {
		.name = "CanRxQueue",
		.cb_mem = &CanRxQueueControlBlock,
		.cb_size = sizeof(CanRxQueueControlBlock),
		.mq_mem = &CanRxQueueBuffer,
		.mq_size = sizeof(CanRxQueueBuffer)};
/* Definitions for QuotaQueue */
osMessageQueueId_t QuotaQueueHandle;
uint8_t QuotaQueueBuffer[1 * 200];
osStaticMessageQDef_t QuotaQueueControlBlock;
const osMessageQueueAttr_t QuotaQueue_attributes = {
		.name = "QuotaQueue",
		.cb_mem = &QuotaQueueControlBlock,
		.cb_size = sizeof(QuotaQueueControlBlock),
		.mq_mem = &QuotaQueueBuffer,
		.mq_size = sizeof(QuotaQueueBuffer)};
/* Definitions for UssdQueue */
osMessageQueueId_t UssdQueueHandle;
uint8_t UssdQueueBuffer[1 * 20];
osStaticMessageQDef_t UssdQueueControlBlock;
const osMessageQueueAttr_t UssdQueue_attributes = {
		.name = "UssdQueue",
		.cb_mem = &UssdQueueControlBlock,
		.cb_size = sizeof(UssdQueueControlBlock),
		.mq_mem = &UssdQueueBuffer,
		.mq_size = sizeof(UssdQueueBuffer)};
/* Definitions for OvdStateQueue */
osMessageQueueId_t OvdStateQueueHandle;
uint8_t OvdStateQueueBuffer[1 * sizeof(uint8_t)];
osStaticMessageQDef_t OvdStateQueueControlBlock;
const osMessageQueueAttr_t OvdStateQueue_attributes = {
		.name = "OvdStateQueue",
		.cb_mem = &OvdStateQueueControlBlock,
		.cb_size = sizeof(OvdStateQueueControlBlock),
		.mq_mem = &OvdStateQueueBuffer,
		.mq_size = sizeof(OvdStateQueueBuffer)};
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
/* Definitions for EepromRecMutex */
osMutexId_t EepromRecMutexHandle;
osStaticMutexDef_t EepromRecMutexControlBlock;
const osMutexAttr_t EepromRecMutex_attributes = {
		.name = "EepromRecMutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = &EepromRecMutexControlBlock,
		.cb_size = sizeof(EepromRecMutexControlBlock),
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
void StartMemsTask(void *argument);
void StartRemoteTask(void *argument);
void StartFingerTask(void *argument);
void StartAudioTask(void *argument);
void StartCanRxTask(void *argument);
void StartCanTxTask(void *argument);
void StartGateTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */
	/* Create the mutex(es) */
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

	/* creation of EepromRecMutex */
	EepromRecMutexHandle = osMutexNew(&EepromRecMutex_attributes);

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
	CommandQueueHandle =
			osMessageQueueNew(1, sizeof(command_t), &CommandQueue_attributes);

	/* creation of ResponseQueue */
	ResponseQueueHandle =
			osMessageQueueNew(1, sizeof(response_t), &ResponseQueue_attributes);

	/* creation of ReportQueue */
	ReportQueueHandle =
			osMessageQueueNew(10, sizeof(report_t), &ReportQueue_attributes);

	/* creation of DriverQueue */
	DriverQueueHandle =
			osMessageQueueNew(1, sizeof(uint8_t), &DriverQueue_attributes);

	/* creation of CanRxQueue */
	CanRxQueueHandle =
			osMessageQueueNew(10, sizeof(can_rx_t), &CanRxQueue_attributes);

	/* creation of QuotaQueue */
	QuotaQueueHandle = osMessageQueueNew(1, 200, &QuotaQueue_attributes);

	/* creation of UssdQueue */
	UssdQueueHandle = osMessageQueueNew(1, 20, &UssdQueue_attributes);

	/* creation of OvdStateQueue */
	OvdStateQueueHandle =
			osMessageQueueNew(1, sizeof(uint8_t), &OvdStateQueue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of ManagerTask */
	ManagerTaskHandle =
			osThreadNew(StartManagerTask, NULL, &ManagerTask_attributes);

	/* creation of NetworkTask */
	NetworkTaskHandle =
			osThreadNew(StartNetworkTask, NULL, &NetworkTask_attributes);

	/* creation of ReporterTask */
	ReporterTaskHandle =
			osThreadNew(StartReporterTask, NULL, &ReporterTask_attributes);

	/* creation of CommandTask */
	CommandTaskHandle =
			osThreadNew(StartCommandTask, NULL, &CommandTask_attributes);

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
void StartManagerTask(void *argument) {
	/* USER CODE BEGIN StartManagerTask */
	uint32_t tick;

	// Initiate, this task get executed first!
	VCU_Init();
	NODE_Init();

	// Peripheral Initiate
	EE_Init();
	BAT_Init();
	RTC_Init();

	// Threads management:
	//		osThreadSuspend(NetworkTaskHandle);
	//		osThreadSuspend(ReporterTaskHandle);
	//		osThreadSuspend(CommandTaskHandle);
	//		osThreadSuspend(MemsTaskHandle);
	//		osThreadSuspend(RemoteTaskHandle);
	//		osThreadSuspend(FingerTaskHandle);
	//		osThreadSuspend(AudioTaskHandle);
	//		osThreadSuspend(CanRxTaskHandle);
	//		osThreadSuspend(CanTxTaskHandle);
	//		osThreadSuspend(GateTaskHandle);

	// Check thread creation
	if (TASK_KernelFailed()) return;

	// Release threads
	osEventFlagsSet(GlobalEventHandle, EVENT_READY);

	/* Infinite loop */
	for (;;) {
		tick = _GetTickMS();
		TASK_IO_SetTick(TASK_MANAGER);

		TASK_CheckStack();
		TASK_CheckWakeup();

		VCU_Refresh();
		NODE_Refresh();

		STATE_Check();

		EE_Refresh();
		IWDG_Refresh();
		osDelayUntil(tick + MANAGER_WAKEUP_MS);
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
void StartNetworkTask(void *argument) {
	/* USER CODE BEGIN StartNetworkTask */
	uint32_t notif;

	TASK_WaitManager();

	// Initiate
	SIM_Init();
	SIM_SetState(SIM_STATE_SERVER_ON, 0);

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_NETWORK);

		if (_osFlagAny(&notif, 100)) {
			if (notif & (FLAG_NET_READ_SMS | FLAG_NET_SEND_USSD)) {
				uint8_t ok = 0;

				if (notif & FLAG_NET_SEND_USSD)
					ok = NET_SendUSSD();
				else if (notif & FLAG_NET_READ_SMS)
					ok = NET_ReadSMS();

				osThreadFlagsSet(CommandTaskHandle,
						ok ? FLAG_COMMAND_OK : FLAG_COMMAND_ERROR);
			}

			if (notif & FLAG_NET_REPORT_DISCARD)
				RPT_IO_PayloadDiscard();
		}

		NET_CheckPayload(PAYLOAD_RESPONSE);
		NET_CheckPayload(PAYLOAD_REPORT);

		NET_CheckCommand();
		NET_CheckClock();
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
void StartReporterTask(void *argument) {
	/* USER CODE BEGIN StartReporterTask */
	uint32_t notif;
	report_t report;

	TASK_WaitManager();
	GPS_Init();

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_REPORTER);

		if (_osFlagAny(&notif, RPT_IntervalDeciderMS(VCU.d.state))) {
			if (notif & FLAG_REPORTER_YIELD) {
				// nothing, just wakeup
			}

			if (notif & FLAG_REPORTER_FLUSH) {
				osMessageQueueReset(ReportQueueHandle);
			}
		}

		// Put report to log
		RPT_ReportCapture(RPT_FrameDecider(), &report);
		while (!_osQueuePut(ReportQueueHandle, &report)) {
			osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_REPORT_DISCARD);
			_DelayMS(100);
		}

		// reset some events group
		EVT_Clr(EVG_NET_SOFT_RESET);
		EVT_Clr(EVG_NET_HARD_RESET);

		GPS_Refresh();
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
void StartCommandTask(void *argument) {
	/* USER CODE BEGIN StartCommandTask */
	command_t cmd;
	response_t resp;

	TASK_WaitManager();

	// Initiate
	CMD_Init();

	// Handle Post-FOTA
	if (FW_ValidResponseIAP()) {
		FW_CaptureResponseIAP(&resp);
		_osQueuePut(ResponseQueueHandle, &resp);
	}

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_COMMAND);

		if (osMessageQueueGet(CommandQueueHandle, &cmd, NULL, osWaitForever) ==
				osOK) {
			EXEC_Command(&cmd, &resp);
			RPT_ResponseCapture(&resp);

			_osQueuePutRst(ResponseQueueHandle, &resp);
			memset(&cmd, 0, sizeof(cmd));
			memset(&resp, 0, sizeof(resp));
		}
	}
	/* USER CODE END StartCommandTask */
}

/* USER CODE BEGIN Header_StartMemsTask */
/**
 * @brief Function implementing the MemsTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMemsTask */
void StartMemsTask(void *argument) {
	/* USER CODE BEGIN StartMemsTask */
	uint32_t notif;
	uint8_t fallen;

	TASK_WaitManager();
	_osFlagOne(&notif, FLAG_MEMS_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	MEMS_Init();

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_MEMS);

		// Check notifications
		if (_osFlagAny(&notif, 1000)) {
			if (notif & FLAG_MEMS_TASK_STOP) {
				EVT_Clr(EVG_BIKE_FALLEN);
				EVT_Clr(EVG_BIKE_MOVED);

				MEMS_DeInit();
				_osFlagOne(&notif, FLAG_MEMS_TASK_START, osWaitForever);
				MEMS_Init();
			}

			if (notif & FLAG_MEMS_DETECTOR_RESET) MEMS_GetRefDetector();

			if (notif & FLAG_MEMS_DETECTOR_TOGGLE) MEMS.det.active = !MEMS.det.active;
		}

		// Read all data
		if (MEMS_Capture()) {
			fallen = MEMS_Process();
			EVT_SetVal(EVG_BIKE_FALLEN, fallen);

			// Drag detector
			if (MEMS.det.active) {
				if (MEMS_Dragged()) EVT_Set(EVG_BIKE_MOVED);
				if (EVT_Get(EVG_BIKE_MOVED))
					osThreadFlagsSet(GateTaskHandle, FLAG_GATE_ALARM_HORN);
			} else
				MEMS_GetRefDetector();
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
void StartRemoteTask(void *argument) {
	/* USER CODE BEGIN StartRemoteTask */
	uint32_t notif, tick, resetTick = 0;
	RMT_CMD command;

	TASK_WaitManager();
	_osFlagOne(&notif, FLAG_REMOTE_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	AES_Init();
	RMT_Init();

	/* Infinite loop */
	for (;;) {
		tick = _GetTickMS();
		TASK_IO_SetTick(TASK_REMOTE);

		if (RMT_Ping(VCU.d.state)) {
			RMT.d.tick.ping = _GetTickMS();
			RMT.d.duration.tx = RMT.d.tick.ping - tick;
		}

		if (_osFlagAny(&notif, 5)) {
			if (notif & FLAG_REMOTE_TASK_STOP) {
				EVT_Set(EVG_REMOTE_MISSING);
				RMT_DeInit();
				_osFlagOne(&notif, FLAG_REMOTE_TASK_START, osWaitForever);
				RMT_Init();
			}

			if (notif & FLAG_REMOTE_RESET) {
				if (!_TickIn(resetTick, RMT_RESET_GUARD_MS)) {
					resetTick = _GetTickMS();
					EVT_Set(EVG_REMOTE_MISSING);
					RMT_DeInit();
					RMT_Init();
				}
			}

			if (notif & FLAG_REMOTE_PAIRING)
				if (RMT_Pairing()) printf("NRF:Pairing Sent\n");

			if (notif & FLAG_REMOTE_RX_IT) {
				if (RMT_ValidateCommand(&command)) {
					RMT.d.tick.rx = _GetTickMS();
					RMT.d.duration.rx = RMT.d.tick.rx - tick;

					if (command == RMT_CMD_PING) {
						if (RMT_GotPairedResponse())
							osThreadFlagsSet(CommandTaskHandle, FLAG_COMMAND_OK);
					} else {
						if (command == RMT_CMD_ANTITHIEF)
							osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_DETECTOR_TOGGLE);

						else if (command == RMT_CMD_ALARM) {
							if (EVT_Get(EVG_BIKE_MOVED))
								osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_DETECTOR_RESET);
							else
								osThreadFlagsSet(GateTaskHandle, FLAG_GATE_ALARM_HORN);
						}

						else if (command == RMT_CMD_SEAT)
							osThreadFlagsSet(GateTaskHandle, FLAG_GATE_OPEN_SEAT);

						_DelayMS(200);
						osThreadFlagsClear(FLAG_REMOTE_RX_IT);
					}
				}
			}
		}

		RMT_Refresh(VCU.d.state);
		RMT.d.duration.full = _GetTickMS() - tick;
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
void StartFingerTask(void *argument) {
	/* USER CODE BEGIN StartFingerTask */
	uint32_t notif;
	uint8_t id;

	TASK_WaitManager();
	_osFlagOne(&notif, FLAG_FINGER_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	FGR_Init();

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_FINGER);

		if (_osFlagAny(&notif, 500)) {
			if (notif & FLAG_FINGER_TASK_STOP) {
				FGR_DeInit();
				_osFlagOne(&notif, FLAG_FINGER_TASK_START, osWaitForever);
				FGR_Init();
			}

			if (VCU.d.state == VEHICLE_STANDBY) {
				if (notif & (FLAG_FINGER_ADD | FLAG_FINGER_DEL | FLAG_FINGER_FETCH |
						FLAG_FINGER_RST)) {
					uint8_t ok = 0;

					if (notif & FLAG_FINGER_ADD) {
						if (FGR_Enroll(&id, &ok)) _osQueuePutRst(DriverQueueHandle, &id);
						osThreadFlagsClear(FLAG_FINGER_PLACED);
					} else if (notif & FLAG_FINGER_DEL) {
						if (_osQueueGet(DriverQueueHandle, &id)) ok = FGR_DeleteID(id);
					} else if (notif & FLAG_FINGER_RST)
						ok = FGR_ResetDB();
					else if (notif & FLAG_FINGER_FETCH)
						ok = FGR_Fetch();

					osThreadFlagsSet(CommandTaskHandle,
							ok ? FLAG_COMMAND_OK : FLAG_COMMAND_ERROR);
				}

				//				if (notif & FLAG_FINGER_PLACED) {
					//					FGR_Authenticate();
					//					osThreadFlagsClear(FLAG_FINGER_PLACED);
				//				}
			}
		}

		GATE_FingerChipPower(VCU.d.state == VEHICLE_STANDBY);
		if (VCU.d.state == VEHICLE_STANDBY) FGR_Authenticate();

		FGR_Verify();
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
void StartAudioTask(void *argument) {
	/* USER CODE BEGIN StartAudioTask */
	uint32_t notif;

	TASK_WaitManager();
	_osFlagOne(&notif, FLAG_AUDIO_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	/* Initiate Wave player (Codec, DMA, I2C) */
	AUDIO_Init();

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_AUDIO);

		if (_osFlagAny(&notif, 1000)) {
			if (notif & FLAG_AUDIO_TASK_STOP) {
				AUDIO_DeInit();
				_osFlagOne(&notif, FLAG_AUDIO_TASK_START, osWaitForever);
				AUDIO_Init();
			}

			// Beep command
			if (notif & FLAG_AUDIO_BEEP) {
				AUDIO_OUT_Pause();
				AUDIO_OUT_SetVolume(100);
				AUDIO_OUT_SetMute(0);
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 1000);
				AUDIO_OUT_Resume();
			}
		}

		AUDIO_OUT_SetMute(VCU.d.state != VEHICLE_RUN);
		AUDIO_OUT_SetVolume(MCU_SpeedToVolume());
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
void StartCanRxTask(void *argument) {
	/* USER CODE BEGIN StartCanRxTask */
	uint32_t notif;
	can_rx_t Rx;

	TASK_WaitManager();
	_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_CANRX);

		// Check notifications
		if (_osFlagOne(&notif, FLAG_CAN_TASK_STOP, 0)) {
			NODE_Init();
			_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
		}

		if (osMessageQueueGet(CanRxQueueHandle, &Rx, NULL, 1000) == osOK) {
			if (Rx.header.IDE == CAN_ID_STD) {
				switch (Rx.header.StdId) {
					case CAND_HMI1:
						HMI1_RX_State(&Rx);
						break;
					case CAND_NODE_DEBUG:
						NODE_RX_Debug(&Rx);
						break;
					case CAND_MCU_CURRENT_DC:
						MCU_RX_CurrentDC(&Rx);
						break;
					case CAND_MCU_VOLTAGE_DC:
						MCU_RX_VoltageDC(&Rx);
						break;
					case CAND_MCU_TORQUE_SPEED:
						MCU_RX_TorqueSpeed(&Rx);
						break;
					case CAND_MCU_FAULT_CODE:
						MCU_RX_FaultCode(&Rx);
						break;
					case CAND_MCU_STATE:
						MCU_RX_State(&Rx);
						break;
					case CAND_MCU_TEMPLATE_R:
						MCU_RX_Template(&Rx);
						break;
					default:
						break;
				}
			} else {
				switch (BMS_CAND(Rx.header.ExtId)) {
					case BMS_CAND(CAND_BMS_PARAM_1):
            		BMS_RX_Param1(&Rx);
					break;
					case BMS_CAND(CAND_BMS_PARAM_2):
            		BMS_RX_Param2(&Rx);
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
void StartCanTxTask(void *argument) {
	/* USER CODE BEGIN StartCanTxTask */
	uint32_t notif, tick100ms, tick1000ms;

	TASK_WaitManager();
	_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
	osThreadFlagsClear(FLAG_MASK);

	// initiate
	CANBUS_Init();

	/* Infinite loop */
	tick100ms = _GetTickMS();
	tick1000ms = _GetTickMS();
	for (;;) {
		TASK_IO_SetTick(TASK_CANTX);

		// Check notifications
		if (_osFlagAny(&notif, 20)) {
			if (notif & FLAG_CAN_TASK_STOP) {
				MCU_PowerOverCAN(0);
				BMS_PowerOverCAN(0);

				CANBUS_DeInit();
				_osFlagOne(&notif, FLAG_CAN_TASK_START, osWaitForever);
				CANBUS_Init();
			}
		}

		// send every 20ms
		if (HMI1.d.active) VCU_TX_SwitchControl();

		// send every 500ms
		if (_TickOut(tick100ms, 100)) {
			tick100ms = _GetTickMS();

			MCU_PowerOverCAN(VCU.d.state == VEHICLE_RUN && BMS.d.run);
			BMS_PowerOverCAN(VCU.d.state >= VEHICLE_READY || MCU.d.run);
		}

		// send every 1000ms
		if (_TickOut(tick1000ms, 1000)) {
			tick1000ms = _GetTickMS();

			if (HMI1.d.active) {
				VCU_TX_Datetime(RTC_Read());
				VCU_TX_ModeData();
			}

			if (MCU.d.active) MCU_SyncCAN();

			if (NODE.d.debug) NODE_DebugCAN();

			VCU_TX_Heartbeat();
		}
	}
	/* USER CODE END StartCanTxTask */
}

/* USER CODE BEGIN Header_StartGateTask */
/**
 * @brief Function implementing the GateTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGateTask */
void StartGateTask(void *argument) {
	/* USER CODE BEGIN StartGateTask */
	uint32_t notif;

	TASK_WaitManager();

	// Initiate
	HBAR_Init();
	HBAR_ReadStates();

	/* Infinite loop */
	for (;;) {
		TASK_IO_SetTick(TASK_GATE);

		// wait forever
		if (_osFlagAny(&notif, 10)) {
			if (notif & FLAG_GATE_HBAR) {
				_DelayMS(100);
				osThreadFlagsClear(FLAG_GATE_HBAR);

				HBAR_ReadStarter(VCU.d.state == VEHICLE_NORMAL);
				if (VCU.d.state >= VEHICLE_STANDBY) HBAR_ReadStates();
			}

			if (notif & FLAG_GATE_ALARM_HORN)
				if (VCU.d.state == VEHICLE_NORMAL) GATE_Horn(500);

			if (notif & FLAG_GATE_OPEN_SEAT)
				if (VCU.d.state == VEHICLE_NORMAL) GATE_Seat(1000);
		}

		HBAR_RefreshSelectSet();

		ML_PredictRange();

		HMI1_Power(VCU.d.state >= VEHICLE_STANDBY);
		MCU_Power12v(VCU.d.state >= VEHICLE_READY);
		GATE_System12v(VCU.d.state >= VEHICLE_STANDBY);
	}
	/* USER CODE END StartGateTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (osKernelGetState() != osKernelRunning) return;

	if (osEventFlagsGet(GlobalEventHandle) != EVENT_READY) return;

	if (GPIO_Pin & INT_REMOTE_IRQ_Pin) {
		if (VCU.d.state >= VEHICLE_NORMAL) RMT_IrqHandler();
	}

	//	if (GPIO_Pin & EXT_FINGER_IRQ_Pin) {
	//		if (VCU.d.state == VEHICLE_STANDBY)
	//			osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_PLACED);
	//	}

	if (!((GPIO_Pin & INT_REMOTE_IRQ_Pin) || (GPIO_Pin & EXT_FINGER_IRQ_Pin))) {
		if (VCU.d.state >= VEHICLE_NORMAL)
			osThreadFlagsSet(GateTaskHandle, FLAG_GATE_HBAR);
	}
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
