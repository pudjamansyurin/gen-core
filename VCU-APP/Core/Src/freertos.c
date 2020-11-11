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
#include "Libs/_command.h"
#include "Libs/_firmware.h"
#include "Libs/_simcom.h"
#include "Libs/_eeprom.h"
#include "Libs/_gyro.h"
#include "Libs/_gps.h"
#include "Libs/_finger.h"
#include "Libs/_audio.h"
#include "Libs/_keyless.h"
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"
#include "Drivers/_canbus.h"
#include "Drivers/_rtc.h"
#include "Drivers/_aes.h"
#include "DMA/_dma_battery.h"
#include "DMA/_dma_simcom.h"
#include "DMA/_dma_ublox.h"
#include "DMA/_dma_finger.h"
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"
#include "iwdg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
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
osEventFlagsId_t GlobalEventHandle;

extern vcu_t VCU;
extern bms_t BMS;
extern hmi1_t HMI1;
extern hmi2_t HMI2;

extern sw_t SW;
extern sim_t SIM;
extern uint32_t AesKey[4];
extern uint16_t BACKUP_VOLTAGE;
/* USER CODE END Variables */
/* Definitions for ManagerTask */
osThreadId_t ManagerTaskHandle;
const osThreadAttr_t ManagerTask_attributes = {
		.name = "ManagerTask",
		.priority = (osPriority_t) osPriorityRealtime7,
		.stack_size = 288 * 4
};
/* Definitions for IotTask */
osThreadId_t IotTaskHandle;
const osThreadAttr_t IotTask_attributes = {
		.name = "IotTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 416 * 4
};
/* Definitions for ReporterTask */
osThreadId_t ReporterTaskHandle;
const osThreadAttr_t ReporterTask_attributes = {
		.name = "ReporterTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 304 * 4
};
/* Definitions for CommandTask */
osThreadId_t CommandTaskHandle;
const osThreadAttr_t CommandTask_attributes = {
		.name = "CommandTask",
		.priority = (osPriority_t) osPriorityAboveNormal,
		.stack_size = 256 * 4
};
/* Definitions for GpsTask */
osThreadId_t GpsTaskHandle;
const osThreadAttr_t GpsTask_attributes = {
		.name = "GpsTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 256 * 4
};
/* Definitions for GyroTask */
osThreadId_t GyroTaskHandle;
const osThreadAttr_t GyroTask_attributes = {
		.name = "GyroTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 304 * 4
};
/* Definitions for KeylessTask */
osThreadId_t KeylessTaskHandle;
const osThreadAttr_t KeylessTask_attributes = {
		.name = "KeylessTask",
		.priority = (osPriority_t) osPriorityAboveNormal,
		.stack_size = 256 * 4
};
/* Definitions for FingerTask */
osThreadId_t FingerTaskHandle;
const osThreadAttr_t FingerTask_attributes = {
		.name = "FingerTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 224 * 4
};
/* Definitions for AudioTask */
osThreadId_t AudioTaskHandle;
const osThreadAttr_t AudioTask_attributes = {
		.name = "AudioTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 240 * 4
};
/* Definitions for SwitchTask */
osThreadId_t SwitchTaskHandle;
const osThreadAttr_t SwitchTask_attributes = {
		.name = "SwitchTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 224 * 4
};
/* Definitions for CanRxTask */
osThreadId_t CanRxTaskHandle;
const osThreadAttr_t CanRxTask_attributes = {
		.name = "CanRxTask",
		.priority = (osPriority_t) osPriorityRealtime,
		.stack_size = 224 * 4
};
/* Definitions for CanTxTask */
osThreadId_t CanTxTaskHandle;
const osThreadAttr_t CanTxTask_attributes = {
		.name = "CanTxTask",
		.priority = (osPriority_t) osPriorityHigh,
		.stack_size = 288 * 4
};
/* Definitions for Hmi2PowerTask */
osThreadId_t Hmi2PowerTaskHandle;
const osThreadAttr_t Hmi2PowerTask_attributes = {
		.name = "Hmi2PowerTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 160 * 4
};
/* Definitions for CommandQueue */
osMessageQueueId_t CommandQueueHandle;
const osMessageQueueAttr_t CommandQueue_attributes = {
		.name = "CommandQueue"
};
/* Definitions for ResponseQueue */
osMessageQueueId_t ResponseQueueHandle;
const osMessageQueueAttr_t ResponseQueue_attributes = {
		.name = "ResponseQueue"
};
/* Definitions for ReportQueue */
osMessageQueueId_t ReportQueueHandle;
const osMessageQueueAttr_t ReportQueue_attributes = {
		.name = "ReportQueue"
};
/* Definitions for DriverQueue */
osMessageQueueId_t DriverQueueHandle;
const osMessageQueueAttr_t DriverQueue_attributes = {
		.name = "DriverQueue"
};
/* Definitions for CanRxQueue */
osMessageQueueId_t CanRxQueueHandle;
const osMessageQueueAttr_t CanRxQueue_attributes = {
		.name = "CanRxQueue"
};
/* Definitions for AudioMutex */
osMutexId_t AudioMutexHandle;
const osMutexAttr_t AudioMutex_attributes = {
		.name = "AudioMutex"
};
/* Definitions for LogMutex */
osMutexId_t LogMutexHandle;
const osMutexAttr_t LogMutex_attributes = {
		.name = "LogMutex"
};
/* Definitions for CanTxMutex */
osMutexId_t CanTxMutexHandle;
const osMutexAttr_t CanTxMutex_attributes = {
		.name = "CanTxMutex"
};
/* Definitions for EepromMutex */
osMutexId_t EepromMutexHandle;
const osMutexAttr_t EepromMutex_attributes = {
		.name = "EepromMutex"
};
/* Definitions for RtcMutex */
osMutexId_t RtcMutexHandle;
const osMutexAttr_t RtcMutex_attributes = {
		.name = "RtcMutex"
};
/* Definitions for CrcMutex */
osMutexId_t CrcMutexHandle;
const osMutexAttr_t CrcMutex_attributes = {
		.name = "CrcMutex"
};
/* Definitions for AesMutex */
osMutexId_t AesMutexHandle;
const osMutexAttr_t AesMutex_attributes = {
		.name = "AesMutex"
};
/* Definitions for SimcomRecMutex */
osMutexId_t SimcomRecMutexHandle;
const osMutexAttr_t SimcomRecMutex_attributes = {
		.name = "SimcomRecMutex",
		.attr_bits = osMutexRecursive,
};
/* Definitions for FingerRecMutex */
osMutexId_t FingerRecMutexHandle;
const osMutexAttr_t FingerRecMutex_attributes = {
		.name = "FingerRecMutex",
		.attr_bits = osMutexRecursive,
};
/* Definitions for KlessRecMutex */
osMutexId_t KlessRecMutexHandle;
const osMutexAttr_t KlessRecMutex_attributes = {
		.name = "KlessRecMutex",
		.attr_bits = osMutexRecursive,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartManagerTask(void *argument);
void StartIotTask(void *argument);
void StartReporterTask(void *argument);
void StartCommandTask(void *argument);
void StartGpsTask(void *argument);
void StartGyroTask(void *argument);
void StartKeylessTask(void *argument);
void StartFingerTask(void *argument);
void StartAudioTask(void *argument);
void StartSwitchTask(void *argument);
void StartCanRxTask(void *argument);
void StartCanTxTask(void *argument);
void StartHmi2PowerTask(void *argument);

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
	/* creation of AudioMutex */
	AudioMutexHandle = osMutexNew(&AudioMutex_attributes);

	/* creation of LogMutex */
	LogMutexHandle = osMutexNew(&LogMutex_attributes);

	/* creation of CanTxMutex */
	CanTxMutexHandle = osMutexNew(&CanTxMutex_attributes);

	/* creation of EepromMutex */
	EepromMutexHandle = osMutexNew(&EepromMutex_attributes);

	/* creation of RtcMutex */
	RtcMutexHandle = osMutexNew(&RtcMutex_attributes);

	/* creation of CrcMutex */
	CrcMutexHandle = osMutexNew(&CrcMutex_attributes);

	/* creation of AesMutex */
	AesMutexHandle = osMutexNew(&AesMutex_attributes);

	/* Create the recursive mutex(es) */
	/* creation of SimcomRecMutex */
	SimcomRecMutexHandle = osMutexNew(&SimcomRecMutex_attributes);

	/* creation of FingerRecMutex */
	FingerRecMutexHandle = osMutexNew(&FingerRecMutex_attributes);

	/* creation of KlessRecMutex */
	KlessRecMutexHandle = osMutexNew(&KlessRecMutex_attributes);

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
	CommandQueueHandle = osMessageQueueNew(1, sizeof(command_t), &CommandQueue_attributes);

	/* creation of ResponseQueue */
	ResponseQueueHandle = osMessageQueueNew(1, sizeof(response_t), &ResponseQueue_attributes);

	/* creation of ReportQueue */
	ReportQueueHandle = osMessageQueueNew(100, sizeof(report_t), &ReportQueue_attributes);

	/* creation of DriverQueue */
	DriverQueueHandle = osMessageQueueNew(1, sizeof(uint8_t), &DriverQueue_attributes);

	/* creation of CanRxQueue */
	CanRxQueueHandle = osMessageQueueNew(10, sizeof(can_rx_t), &CanRxQueue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	GlobalEventHandle = osEventFlagsNew(NULL);
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of ManagerTask */
	ManagerTaskHandle = osThreadNew(StartManagerTask, NULL, &ManagerTask_attributes);

	/* creation of IotTask */
	IotTaskHandle = osThreadNew(StartIotTask, NULL, &IotTask_attributes);

	/* creation of ReporterTask */
	ReporterTaskHandle = osThreadNew(StartReporterTask, NULL, &ReporterTask_attributes);

	/* creation of CommandTask */
	CommandTaskHandle = osThreadNew(StartCommandTask, NULL, &CommandTask_attributes);

	/* creation of GpsTask */
	GpsTaskHandle = osThreadNew(StartGpsTask, NULL, &GpsTask_attributes);

	/* creation of GyroTask */
	GyroTaskHandle = osThreadNew(StartGyroTask, NULL, &GyroTask_attributes);

	/* creation of KeylessTask */
	KeylessTaskHandle = osThreadNew(StartKeylessTask, NULL, &KeylessTask_attributes);

	/* creation of FingerTask */
	FingerTaskHandle = osThreadNew(StartFingerTask, NULL, &FingerTask_attributes);

	/* creation of AudioTask */
	AudioTaskHandle = osThreadNew(StartAudioTask, NULL, &AudioTask_attributes);

	/* creation of SwitchTask */
	SwitchTaskHandle = osThreadNew(StartSwitchTask, NULL, &SwitchTask_attributes);

	/* creation of CanRxTask */
	CanRxTaskHandle = osThreadNew(StartCanRxTask, NULL, &CanRxTask_attributes);

	/* creation of CanTxTask */
	CanTxTaskHandle = osThreadNew(StartCanTxTask, NULL, &CanTxTask_attributes);

	/* creation of Hmi2PowerTask */
	Hmi2PowerTaskHandle = osThreadNew(StartHmi2PowerTask, NULL, &Hmi2PowerTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	HAL_Delay(1000);
	/* USER CODE END RTOS_THREADS */

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

	// Initialization, this task get executed first!
	VCU.Init();
	BMS.Init();
	HMI1.Init();
	HMI2.Init();

	// Peripheral initialization
	CANBUS_Init();
	BAT_DMA_Init();
	EEPROM_Init();

	// Threads management:
	//	osThreadSuspend(IotTaskHandle);
	//	osThreadSuspend(ReporterTaskHandle);
	//	osThreadSuspend(CommandTaskHandle);
	//	osThreadSuspend(GpsTaskHandle);
	//	osThreadSuspend(GyroTaskHandle);
	//	osThreadSuspend(KeylessTaskHandle);
	osThreadSuspend(FingerTaskHandle);
	//	osThreadSuspend(AudioTaskHandle);
	//	osThreadSuspend(SwitchTaskHandle);
	//	osThreadSuspend(CanRxTaskHandle);
	//	osThreadSuspend(CanTxTaskHandle);
	osThreadSuspend(Hmi2PowerTaskHandle);

	// Release threads
	osEventFlagsSet(GlobalEventHandle, EVENT_READY);

	/* Infinite loop */
	for (;;) {
		lastWake = _GetTickMS();

		MX_IWDG_Reset();

		// _DummyDataGenerator();

		// _RTOS_Debugger(1000);

		// BAT_Debugger();

		// Other stuffs
		HMI1.d.status.daylight = RTC_IsDaylight(VCU.d.rtc.timestamp);
		HMI1.d.status.warning = BMS.d.warning;
		HMI1.d.status.overheat = BMS.d.overheat;

		osDelayUntil(lastWake + 1000);
	}
	/* USER CODE END StartManagerTask */
}

/* USER CODE BEGIN Header_StartIotTask */
/**
 * @brief Function implementing the IotTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartIotTask */
void StartIotTask(void *argument)
{
	/* USER CODE BEGIN StartIotTask */
	TickType_t lastWake;
	report_t report;
	payload_t pReport = {
			.type = PAYLOAD_REPORT,
			.pQueue = &ReportQueueHandle,
			.pPayload = &report
	};
	response_t response;
	payload_t pResponse = {
			.type = PAYLOAD_RESPONSE,
			.pQueue = &ResponseQueueHandle,
			.pPayload = &response
	};

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Start simcom module
	SIMCOM_DMA_Init();
	Simcom_SetState(SIM_STATE_SERVER_ON, 0);

	/* Infinite loop */
	for (;;) {
		lastWake = _GetTickMS();

		// Upload Report
		if (Packet_Pending(&pReport))
			if (!Send_Payload(&pReport))
				Simcom_SetState(SIM_STATE_SERVER_ON, 0);

		// Upload Response
		if (Packet_Pending(&pResponse))
			if (!Send_Payload(&pResponse))
				Simcom_SetState(SIM_STATE_SERVER_ON, 0);

		// ================= SIMCOM Related Routines ================
		if (RTC_NeedCalibration())
			if (Simcom_SetState(SIM_STATE_READY, 0))
				RTC_CalibrateWithSimcom();

		Simcom_UpdateSignalQuality();

		osDelayUntil(lastWake + 1000);
	}
	/* USER CODE END StartIotTask */
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
	report_t report;
	FRAME_TYPE frame;
	osStatus_t status;
	TickType_t lastWake;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	Report_Init(FR_SIMPLE, &report);

	/* Infinite loop */
	for (;;) {
		lastWake = _GetTickMS();

		frame = Frame_Decider();

		Report_Capture(frame, &report);

		// Put report to log
		do {
			status = osMessageQueuePut(ReportQueueHandle, &report, 0U, 0U);
			// already full, remove oldest
			if (status == osErrorResource)
				osThreadFlagsSet(IotTaskHandle, EVT_IOT_DISCARD);
		} while (status != osOK);

		// reset some events group
		VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 0);
		VCU.SetEvent(EV_VCU_NET_HARD_RESET, 0);

		// TODO: DELETE_ME
		VCU.d.interval = 5;
		osDelayUntil(lastWake + (VCU.d.interval * 1000));
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
	uint8_t driver;
	command_t command;
	response_t response;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialize
	Response_Init(&response);

	// Handle Post-FOTA
	FW_PostFota(&response);

	/* Infinite loop */
	for (;;) {
		// get command in queue
		if (osMessageQueueGet(CommandQueueHandle, &command, NULL, osWaitForever) == osOK) {
			Command_Debugger(&command);

			// default command response
			response.data.code = RESPONSE_STATUS_OK;
			strcpy(response.data.message, "");

			// handle the command
			if (command.data.code == CMD_CODE_GEN) {
				switch (command.data.sub_code) {
					case CMD_GEN_INFO :
						CMD_GenInfo(&response);
						break;

					case CMD_GEN_LED :
						CMD_GenLed(&command);
						break;

					case CMD_GEN_KNOB :
						CMD_GenKnob(&command);
						break;

					case CMD_GEN_UPGRADE_VCU :
						CMD_GenUpgrade(&command, &response);
						break;

					case CMD_GEN_UPGRADE_HMI :
						CMD_GenUpgrade(&command, &response);
						break;

					default:
						response.data.code = RESPONSE_STATUS_INVALID;
						break;
				}
			}

			else if (command.data.code == CMD_CODE_REPORT) {
				switch (command.data.sub_code) {
					case CMD_REPORT_RTC :
						CMD_ReportRTC(&command);
						break;

					case CMD_REPORT_ODOM :
						CMD_ReportOdom(&command);
						break;

					case CMD_REPORT_UNITID :
						CMD_ReportUnitID(&command);
						break;

					default:
						response.data.code = RESPONSE_STATUS_INVALID;
						break;
				}
			}

			else if (command.data.code == CMD_CODE_AUDIO) {
				switch (command.data.sub_code) {
					case CMD_AUDIO_BEEP :
						CMD_AudioBeep();
						break;

					case CMD_AUDIO_MUTE :
						CMD_AudioMute(&command);
						break;

					case CMD_AUDIO_VOL :
						CMD_AudioVol(&command);
						break;

					default:
						response.data.code = RESPONSE_STATUS_INVALID;
						break;
				}
			}

			else if (command.data.code == CMD_CODE_FINGER) {
				// put finger index to queue
				driver = command.data.value;
				osMessageQueuePut(DriverQueueHandle, &driver, 0U, 0U);

				switch (command.data.sub_code) {
					case CMD_FINGER_ADD :
						CMD_Finger(EVT_FINGER_ADD, &response);
						break;

					case CMD_FINGER_DEL :
						CMD_Finger(EVT_FINGER_DEL, &response);
						break;

					case CMD_FINGER_RST :
						CMD_Finger(EVT_FINGER_RST, &response);
						break;

					default:
						response.data.code = RESPONSE_STATUS_INVALID;
						break;
				}
			}

			else if (command.data.code == CMD_CODE_KEYLESS) {
				switch (command.data.sub_code) {
					case CMD_KEYLESS_PAIRING :
						CMD_KeylessPairing(&response);
						break;

					default:
						response.data.code = RESPONSE_STATUS_INVALID;
						break;
				}
			}

			else
				response.data.code = RESPONSE_STATUS_INVALID;

			// Get current snapshot
			Response_Capture(&response);
			osMessageQueuePut(ResponseQueueHandle, &response, 0U, 0U);
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
	TickType_t lastWake;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialize
	UBLOX_DMA_Init();
	GPS_Init();

	/* Infinite loop */
	for (;;) {
		lastWake = _GetTickMS();

		GPS_Capture();
		GPS_CalculateOdometer();
		// GPS_Debugger();

		osDelayUntil(lastWake + (GPS_INTERVAL * 1000));
	}
	/* USER CODE END StartGpsTask */
}

/* USER CODE BEGIN Header_StartGyroTask */
/**
 * @brief Function implementing the GyroTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGyroTask */
void StartGyroTask(void *argument)
{
	/* USER CODE BEGIN StartGyroTask */
	uint32_t flag;
	TickType_t lastWake;
	mems_decision_t decider;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* MPU6050 Initialization*/
	GYRO_Init();

	/* Infinite loop */
	for (;;) {
		lastWake = _GetTickMS();

		// Read all accelerometer, gyroscope (average)
		decider = GYRO_Decision(50);
		// Gyro_Debugger(&decider);

		// Check accelerometer, happens when impact detected
		if (VCU.ReadEvent(EV_VCU_BIKE_CRASHED) != decider.crash.state)
			VCU.SetEvent(EV_VCU_BIKE_CRASHED, decider.crash.state);

		// Check gyroscope, happens when fall detected
		if (VCU.ReadEvent(EV_VCU_BIKE_FALLING) != decider.fall.state)
			VCU.SetEvent(EV_VCU_BIKE_FALLING, decider.fall.state);

		// Handle for both event
		if (decider.crash.state || decider.fall.state) {
			// Turn OFF BMS (+ MCU)

			// indicators
			_LedWrite(decider.fall.state);
			flag = decider.fall.state ? EVT_AUDIO_BEEP_START : EVT_AUDIO_BEEP_STOP;
			osThreadFlagsSet(AudioTaskHandle, flag);
		}

		osDelayUntil(lastWake + 100);
	}
	/* USER CODE END StartGyroTask */
}

/* USER CODE BEGIN Header_StartKeylessTask */
/**
 * @brief Function implementing the KeylessTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartKeylessTask */
void StartKeylessTask(void *argument)
{
	/* USER CODE BEGIN StartKeylessTask */
	uint32_t notif;
	RF_CMD command;
	uint32_t tick_pairing = 0;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// initialization
	AES_Init();
	RF_Init();

	// osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_PAIRING);
	/* Infinite loop */
	for (;;) {
		// Check response
		if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 3)) {
			// handle reset key & id
			if (notif & EVT_KEYLESS_RESET) {
				AES_Init();
				RF_Init();
			}

			// handle Pairing
			if (notif & EVT_KEYLESS_PAIRING) {
				RF_Pairing();
				tick_pairing = _GetTickMS();
			}

			// handle incoming payload
			if (notif & EVT_KEYLESS_RX_IT) {
				RF_Debugger();

				// process
				if (RF_ValidateCommand(&command)) {
					// response pairing command
					if (tick_pairing > 0) {
						if (_GetTickMS() - tick_pairing < 5000)
							osThreadFlagsSet(CommandTaskHandle, EVT_COMMAND_OK);
						tick_pairing = 0;
					}

					// handle command
					switch (command) {
						case RF_CMD_PING:
							LOG_StrLn("NRF:Command = PING");

							// update heart-beat
							VCU.d.tick.keyless = _GetTickMS();
							break;

						case RF_CMD_ALARM:
							LOG_StrLn("NRF:Command = ALARM");

							// toggle Hazard & HORN (+ Sein Lamp)
							//							for (uint8_t i = 0; i < 2; i++) {
							//								SW.runner.hazard = 1;
							//								HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, 1);
							//								_DelayMS(200);
							//								SW.runner.hazard = 0;
							//								HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, 0);
							//								_DelayMS(100);
							//							}
							break;

						case RF_CMD_SEAT:
							LOG_StrLn("NRF:Command = SEAT");

							//							// open the seat via solenoid
							//							HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, 1);
							//							_DelayMS(100);
							//							HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, 0);
							break;

						default:
							break;
					}

					// valid command indicator
					//					osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_START);
					//					for (uint8_t i = 0; i < (command + 1); i++) {
					//						_LedToggle();
					//
					//						_DelayMS((command + 1) * 50);
					//					}
					//					_LedWrite(0);
					//					osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_STOP);
				}
			}
			osThreadFlagsClear(EVT_MASK);
		}

		RF_SendPing(1);
		RF_Refresh();
	}
	/* USER CODE END StartKeylessTask */
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
	uint8_t driver, p;
	int8_t id;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialisation
	FINGER_DMA_Init();
	Finger_Init();

	/* Infinite loop */
	for (;;) {
		// check if user put finger
		if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 100)) {
			if (notif & EVT_FINGER_PLACED) {
				id = Finger_AuthFast();
				// Finger is registered
				if (id >= 0) {
					// FIXME: use vehicle_state
					VCU.d.state.knob = !VCU.d.state.knob;

					// Finger Heart-Beat
					if (!VCU.d.state.knob)
						id = DRIVER_ID_NONE;

					VCU.d.driver_id = id;

					// Handle bounce effect
					_DelayMS(5000);
				}
			}

			if (notif & (EVT_FINGER_ADD | EVT_FINGER_DEL | EVT_FINGER_RST)) {
				// get driver value
				if (osMessageQueueGet(DriverQueueHandle, &driver, NULL, 0U) == osOK) {
					if (notif & EVT_FINGER_ADD)
						p = Finger_Enroll(driver);
					else if (notif & EVT_FINGER_DEL)
						p = Finger_DeleteID(driver);
					else if (notif & EVT_FINGER_RST)
						p = Finger_EmptyDatabase();

					// handle response
					osThreadFlagsSet(CommandTaskHandle, p ? EVT_COMMAND_OK : EVT_COMMAND_ERROR);
				}
			}

			// reset pending flag
			osThreadFlagsClear(EVT_MASK);
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

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Initialize Wave player (Codec, DMA, I2C) */
	AUDIO_Init();
	AUDIO_Play();

	/* Infinite loop */
	for (;;) {
		// wait with timeout
		if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 100)) {
			// Beep command
			if (notif & EVT_AUDIO_BEEP) {
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
				_DelayMS(250);
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
			}

			// Long-Beep Command
			if (notif & EVT_AUDIO_BEEP_START)
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 0);
			if (notif & EVT_AUDIO_BEEP_STOP)
				AUDIO_BeepStop();

			// Mute command
			if (notif & EVT_AUDIO_MUTE_ON)
				AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
			if (notif & EVT_AUDIO_MUTE_OFF)
				AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);

		}

		// update volume
		AUDIO_OUT_SetVolume(VCU.d.volume);
		//        AUDIO_OUT_SetVolume(10);
	}
	/* USER CODE END StartAudioTask */
}

/* USER CODE BEGIN Header_StartSwitchTask */
/**
 * @brief Function implementing the SwitchTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartSwitchTask */
void StartSwitchTask(void *argument)
{
	/* USER CODE BEGIN StartSwitchTask */
	uint32_t notif;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialise
	HBAR_ReadStates();

	// Check GPIOs state
	VCU.CheckMain5vPower();
	// VCU.d.state.knob = HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);

	/* Infinite loop */
	for (;;) {
		// wait forever
		if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny | osFlagsNoClear, 500)) {
			osThreadFlagsClear(EVT_MASK);
			// handle bounce effect
			_DelayMS(50);

			// Handle switch EXTI interrupt
			if (notif & EVT_SWITCH_TRIGGERED) {
				HBAR_ReadStates();
				HBAR_TimerSelectSet();
				HBAR_RunSelectOrSet();
			}

			// Handle other EXTI interrupt
			// BMS Power IRQ
			if (notif & EVT_SWITCH_REG_5V_IRQ) {
				// independent mode activated

			}
			// Starter Button IRQ
			if (notif & EVT_SWITCH_STARTER_IRQ) {
				// check KNOB, KickStand, Keyless, Fingerprint

			}
			// KNOB IRQ
			if (notif & EVT_SWITCH_KNOB_IRQ) {
				// get current state
				// VCU.d.state.knob = HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);
			}
		}

		// Check REG_5V power state
		VCU.CheckMain5vPower();
	}
	/* USER CODE END StartSwitchTask */
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
	can_rx_t Rx;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	for (;;) {
		if (osMessageQueueGet(CanRxQueueHandle, &Rx, NULL, osWaitForever) == osOK) {
			// handle STD message
			switch (CANBUS_ReadID(&(Rx.header))) {
				case CAND_HMI1 :
					HMI1.can.r.State(&Rx);
					break;
				case CAND_HMI2 :
					HMI2.can.r.State(&Rx);
					break;
				default:
					// BMS - Extended ID
					switch (_R(CANBUS_ReadID(&(Rx.header)), 20)) {
						case CAND_BMS_PARAM_1 :
							BMS.can.r.Param1(&Rx);
							break;
						case CAND_BMS_PARAM_2 :
							BMS.can.r.Param2(&Rx);
							break;
						default:
							break;
					}

					break;
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
	TickType_t lastWake, last500ms, last1000ms;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	last500ms = _GetTickMS();
	last1000ms = _GetTickMS();
	for (;;) {
		lastWake = _GetTickMS();

		// send every 20m
		VCU.can.t.SwitchModeControl(&SW);

		// send every 500ms
		if (lastWake - last500ms > 500) {
			last500ms = _GetTickMS();

			VCU.can.t.MixedData(&(SW.runner));
		}

		// send every 1000ms
		if (lastWake - last1000ms > 1000) {
			last1000ms = _GetTickMS();

			VCU.can.t.Datetime(&(VCU.d.rtc.timestamp));
			VCU.can.t.SubTripData(&(SW.runner.mode.sub.trip[0]));
		}

		// Handle Knob Changes
		HMI1.Power(VCU.d.state.knob);
		HMI2.PowerOverCan(VCU.d.state.knob);
		BMS.PowerOverCan(VCU.d.state.knob);

		// Refresh state
		HMI1.Refresh();
		HMI2.Refresh();
		BMS.RefreshIndex();

		osDelayUntil(lastWake + 20);
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

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	for (;;) {
		if (_osThreadFlagsWait(&notif, EVT_HMI2POWER_CHANGED, osFlagsWaitAny, osWaitForever)) {

			if (HMI2.d.power)
				while (!HMI2.d.started)
					HMI2.PowerOn();

			else
				while (HMI2.d.started)
					HMI2.PowerOff();

		}
	}
	/* USER CODE END StartHmi2PowerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (osKernelGetState() == osKernelRunning) {
		// handle BMS_IRQ (is 5v exist?)
		if (GPIO_Pin == EXT_REG_5V_IRQ_Pin)
			osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_REG_5V_IRQ);

		// handle Starter Button
		if (GPIO_Pin == EXT_STARTER_IRQ_Pin)
			osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_STARTER_IRQ);

		// handle KNOB IRQ (Power control for HMI1 & HMI2)
		if (GPIO_Pin == EXT_KNOB_IRQ_Pin)
			osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_KNOB_IRQ);

		// handle Finger IRQ
		if (GPIO_Pin == EXT_FINGER_IRQ_Pin)
			osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_PLACED);

		// handle NRF24 IRQ
		if (GPIO_Pin == INT_KEYLESS_IRQ_Pin)
			RF_IrqHandler();

		// handle Switches EXTI
		for (uint8_t i = 0; i < SW_K_TOTAL; i++)
			if (GPIO_Pin == SW.list[i].pin) {
				osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_TRIGGERED);
				break;
			}

	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
