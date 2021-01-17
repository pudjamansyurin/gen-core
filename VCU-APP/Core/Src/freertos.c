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
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "rtc.h"
#include "iwdg.h"
#include "can.h"
#include "aes.h"
#include "usart.h"

#include "DMA/_dma_ublox.h"
#include "DMA/_dma_finger.h"

#include "Drivers/_canbus.h"
#include "Drivers/_rtc.h"
#include "Drivers/_aes.h"
#include "Drivers/_bat.h"
#include "Drivers/_simcom.h"

#include "Libs/_command.h"
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"
#include "Libs/_gyro.h"
#include "Libs/_gps.h"
#include "Libs/_finger.h"
#include "Libs/_audio.h"
#include "Libs/_remote.h"
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"

#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"
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
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_uart4_rx;

/* USER CODE END Variables */
/* Definitions for ManagerTask */
osThreadId_t ManagerTaskHandle;
const osThreadAttr_t ManagerTask_attributes = {
  .name = "ManagerTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 256 * 4
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
/* Definitions for RemoteTask */
osThreadId_t RemoteTaskHandle;
const osThreadAttr_t RemoteTask_attributes = {
  .name = "RemoteTask",
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
/* Definitions for CanRxTask */
osThreadId_t CanRxTaskHandle;
const osThreadAttr_t CanRxTask_attributes = {
  .name = "CanRxTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 229 * 4
};
/* Definitions for CanTxTask */
osThreadId_t CanTxTaskHandle;
const osThreadAttr_t CanTxTask_attributes = {
  .name = "CanTxTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 230 * 4
};
/* Definitions for Hmi2PowerTask */
osThreadId_t Hmi2PowerTaskHandle;
const osThreadAttr_t Hmi2PowerTask_attributes = {
  .name = "Hmi2PowerTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 176 * 4
};
/* Definitions for GateTask */
osThreadId_t GateTaskHandle;
const osThreadAttr_t GateTask_attributes = {
  .name = "GateTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 224 * 4
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
/* Definitions for LogRecMutex */
osMutexId_t LogRecMutexHandle;
const osMutexAttr_t LogRecMutex_attributes = {
  .name = "LogRecMutex",
  .attr_bits = osMutexRecursive,
};
/* Definitions for GlobalEvent */
osEventFlagsId_t GlobalEventHandle;
const osEventFlagsAttr_t GlobalEvent_attributes = {
  .name = "GlobalEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void CheckTaskState(rtos_task_t *rtos);
static void CheckVehicleState(void);
/* USER CODE END FunctionPrototypes */

void StartManagerTask(void *argument);
void StartIotTask(void *argument);
void StartReporterTask(void *argument);
void StartCommandTask(void *argument);
void StartGpsTask(void *argument);
void StartGyroTask(void *argument);
void StartRemoteTask(void *argument);
void StartFingerTask(void *argument);
void StartAudioTask(void *argument);
void StartCanRxTask(void *argument);
void StartCanTxTask(void *argument);
void StartHmi2PowerTask(void *argument);
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
  /* creation of EepromMutex */
  EepromMutexHandle = osMutexNew(&EepromMutex_attributes);

  /* creation of RtcMutex */
  RtcMutexHandle = osMutexNew(&RtcMutex_attributes);

  /* creation of CrcMutex */
  CrcMutexHandle = osMutexNew(&CrcMutex_attributes);

  /* creation of AesMutex */
  AesMutexHandle = osMutexNew(&AesMutex_attributes);

  /* Create the recursive mutex(es) */
  /* creation of LogRecMutex */
  LogRecMutexHandle = osMutexNew(&LogRecMutex_attributes);

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
  ReportQueueHandle = osMessageQueueNew (60, sizeof(report_t), &ReportQueue_attributes);

  /* creation of DriverQueue */
  DriverQueueHandle = osMessageQueueNew (1, sizeof(uint8_t), &DriverQueue_attributes);

  /* creation of CanRxQueue */
  CanRxQueueHandle = osMessageQueueNew (10, sizeof(can_rx_t), &CanRxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
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
  HAL_Delay(1000);
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

  // Initialization, this task get executed first!
  VCU.Init();
  BMS.Init();
  HMI1.Init();
  HMI2.Init();

  // Peripheral initialization
  RTC_Init(&hrtc);
  EEPROM_Init(&hi2c2);
  CANBUS_Init(&hcan1);
  BAT_Init(&hadc1);

  // Threads management:
  //  osThreadSuspend(IotTaskHandle);
  //  osThreadSuspend(ReporterTaskHandle);
  //  osThreadSuspend(CommandTaskHandle);
  //  osThreadSuspend(GpsTaskHandle);
  //  osThreadSuspend(GyroTaskHandle);
  //  osThreadSuspend(RemoteTaskHandle);
  //  osThreadSuspend(FingerTaskHandle);
  //  osThreadSuspend(AudioTaskHandle);
  //  osThreadSuspend(CanRxTaskHandle);
  //  osThreadSuspend(CanTxTaskHandle);
  osThreadSuspend(Hmi2PowerTaskHandle);
  //  osThreadSuspend(GateTaskHandle);

  // Release threads
  osEventFlagsSet(GlobalEventHandle, EVENT_READY);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.manager.wakeup = _GetTickMS() / 1000;
    lastWake = _GetTickMS();

    // Other stuffs
    VCU.d.state.error = BMS.d.error || VCU.ReadEvent(EV_VCU_BIKE_FALLEN);
    HMI1.d.state.overheat = BMS.d.overheat;
    HMI1.d.state.daylight = RTC_IsDaylight(VCU.d.rtc.timestamp);
    HMI1.d.state.warning = BMS.d.warning || VCU.d.state.error;

    // Vehicle states
    CheckVehicleState();

    CheckTaskState(&(VCU.d.task));

    // _DummyDataGenerator();
    BAT_ScanValue(&(VCU.d.bat));
    MX_IWDG_Reset();

    osDelayUntil(lastWake + 1111);
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
      .pPayload = &report,
      .pending = 0
  };
  response_t response;
  payload_t pResponse = {
      .type = PAYLOAD_RESPONSE,
      .pQueue = &ResponseQueueHandle,
      .pPayload = &response,
      .pending = 0
  };

  // wait until ManagerTask done
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

  // Initialize
  Simcom_Init(&huart1, &hdma_usart1_rx);
  Simcom_SetState(SIM_STATE_READY, 0);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.iot.wakeup = _GetTickMS() / 1000;
    lastWake = _GetTickMS();

    // Upload Response
    if (RPT_PacketPending(&pResponse))
      if (!RPT_SendPayload(&pResponse))
        Simcom_SetState(SIM_STATE_SERVER_ON, 0);

    // Upload Report
    if (RPT_PacketPending(&pReport))
      if (!RPT_SendPayload(&pReport))
        Simcom_SetState(SIM_STATE_SERVER_ON, 0);

    // SIMCOM Related Routines
    if (RTC_NeedCalibration(&(VCU.d.rtc)))
      if (Simcom_SetState(SIM_STATE_READY, 0))
        RTC_CalibrateWithSimcom(&(VCU.d.rtc));

    Simcom_UpdateSignalQuality();

    osDelayUntil(lastWake + 500);
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

  // Initialize
  RPT_ReportInit(FR_SIMPLE, &report, &(VCU.d.seq_id.report));

  /* Infinite loop */
  for (;;) {
    VCU.d.task.reporter.wakeup = _GetTickMS() / 1000;
    lastWake = _GetTickMS();

    frame = RPT_FrameDecider(!VCU.d.gpio.power5v);
    RPT_ReportCapture(frame, &report, &(VCU.d), &(BMS.d), &(HBAR.runner.mode.d));

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
  RPT_ResponseInit(&response, &(VCU.d.seq_id.response));

  // Handle Post-FOTA
  if (FW_PostFota(&response, &(VCU.d.unit_id), &(HMI1.d.version)))
    osMessageQueuePut(ResponseQueueHandle, &response, 0U, 0U);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.command.wakeup = _GetTickMS() / 1000;
    // get command in queue
    if (osMessageQueueGet(CommandQueueHandle, &command, NULL, osWaitForever) == osOK) {
      // Command_Debugger(&command);

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

          case CMD_GEN_OVERRIDE :
            CMD_GenOverride(&command, &(VCU.d.state.override));
            break;

          case CMD_GEN_FOTA_VCU:
            CMD_GenFota(IAP_VCU, &response, &(VCU.d.bat), &(HMI1.d.version));
            break;

          case CMD_GEN_FOTA_HMI:
            CMD_GenFota(IAP_HMI, &response, &(VCU.d.bat), &(HMI1.d.version));
            break;

          default:
            response.data.code = RESPONSE_STATUS_INVALID;
            break;
        }
      }

      else if (command.data.code == CMD_CODE_REPORT) {
        switch (command.data.sub_code) {
          case CMD_REPORT_RTC :
            CMD_ReportRTC(&command, &(VCU.d.rtc));
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

      else if (command.data.code == CMD_CODE_REMOTE) {
        switch (command.data.sub_code) {
          case CMD_REMOTE_PAIRING :
            CMD_RemotePairing(&response);
            break;

          default:
            response.data.code = RESPONSE_STATUS_INVALID;
            break;
        }
      }

      else
        response.data.code = RESPONSE_STATUS_INVALID;

      // Get current snapshot
      RPT_ResponseCapture(&response, &(VCU.d.unit_id));
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
  uint32_t lastWake;
  uint8_t movement;

  // wait until ManagerTask done
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

  // Initialize
  GPS_Init(&huart2, &hdma_usart2_rx);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.gps.wakeup = _GetTickMS() / 1000;
    lastWake = _GetTickMS();

    GPS_Capture(&(VCU.d.gps));

    VCU.d.speed = GPS_CalculateSpeed(&(VCU.d.gps));

    movement = GPS_CalculateOdometer(&(VCU.d.gps));
    if (movement)
      VCU.SetOdometer(movement);

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
  uint32_t notif, flag;
  movement_t movement;

  // wait until ManagerTask done
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);
  _osThreadFlagsWait(&notif, EVT_GYRO_TASK_START, osFlagsWaitAll, osWaitForever);
  osThreadFlagsClear(EVT_AUDIO_TASK_STOP);

  /* MPU6050 Initialization*/
  GYRO_Init(&hi2c3);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.gyro.wakeup = _GetTickMS() / 1000;

    // Check notifications
    if (_osThreadFlagsWait(&notif, EVT_GYRO_TASK_STOP, osFlagsWaitAll, 1000)) {
      GYRO_DeInit();
      _osThreadFlagsWait(&notif, EVT_GYRO_TASK_START, osFlagsWaitAll, osWaitForever);
      GYRO_Init(&hi2c3);
    }

    // Read all accelerometer, gyroscope (average)
    GYRO_Decision(&movement, &(VCU.d.motion), 50);

    VCU.SetEvent(EV_VCU_BIKE_FALLEN, movement.fallen);
    // Indicators
    GATE_LedWrite(movement.fallen);
    flag = movement.fallen ? EVT_AUDIO_BEEP_START : EVT_AUDIO_BEEP_STOP;
    osThreadFlagsSet(AudioTaskHandle, flag);
  }
  /* USER CODE END StartGyroTask */
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

  // wait until needed
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);
  _osThreadFlagsWait(&notif, EVT_REMOTE_TASK_START, osFlagsWaitAll, osWaitForever);
  osThreadFlagsClear(EVT_AUDIO_TASK_STOP);

  // initialization
  AES_Init(&hcryp);
  RMT_Init(&(VCU.d.unit_id), &hspi1);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.remote.wakeup = _GetTickMS() / 1000;

    // Check response
    if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 3)) {
      // handle threads management
      if (notif & EVT_REMOTE_TASK_STOP) {
        RMT_DeInit();
        _osThreadFlagsWait(&notif, EVT_REMOTE_TASK_START, osFlagsWaitAll, osWaitForever);
        RMT_Init(&(VCU.d.unit_id), &hspi1);
      }

      // handle Pairing
      if (notif & EVT_REMOTE_PAIRING)
        RMT_Pairing(&(VCU.d.unit_id));

      // handle incoming payload
      if (notif & EVT_REMOTE_RX_IT) {
        if (RMT_GotPairedResponse())
          osThreadFlagsSet(CommandTaskHandle, EVT_COMMAND_OK);

        // process command
        if (RMT_ValidateCommand(&command)) {
          if (command == RMT_CMD_PING)
            printf("NRF:Command = PING\n");
          else if (command == RMT_CMD_SEAT) {
            printf("NRF:Command = SEAT\n");

            GATE_SeatToggle();
            // FIXME: use real starter button
            VCU.d.gpio.starter = 1;
          }
          else if (command == RMT_CMD_ALARM) {
            printf("NRF:Command = ALARM\n");

            for (uint8_t i = 0; i < 2; i++)
              GATE_HornToggle(&(HBAR.runner.hazard));
          }
        }
      }
      // osThreadFlagsClear(EVT_MASK);
    }

    RMT_Ping(&(HMI1.d.state.unremote));
//    if (VCU.d.state.vehicle >= VEHICLE_NORMAL)
//      if (RMT_NeedReset())
//        RMT_Init(&(VCU.d.unit_id), &hspi1);

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
  int8_t id;
  uint32_t notif;
  uint8_t driver, p = 0;

  // wait until ManagerTask done
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);
  _osThreadFlagsWait(&notif, EVT_FINGER_TASK_START, osFlagsWaitAll, osWaitForever);
  osThreadFlagsClear(EVT_AUDIO_TASK_STOP);

  // Initialisation
  FINGER_Init(&huart4, &hdma_uart4_rx);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.finger.wakeup = _GetTickMS() / 1000;

    // check if user put finger
    if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 1000)) {
      // handle reset
      if (notif & EVT_FINGER_TASK_STOP) {
        FINGER_DeInit();
        _osThreadFlagsWait(&notif, EVT_FINGER_TASK_START, osFlagsWaitAll, osWaitForever);
        FINGER_Init(&huart4, &hdma_uart4_rx);
      }

      if (notif & EVT_FINGER_PLACED) {
        id = FINGER_AuthFast();
        // Finger is registered
        if (id >= 0) {
          if (HMI1.d.state.unfinger)
            VCU.SetDriver(id);
          else
            VCU.SetDriver(DRIVER_ID_NONE);
        }

        // Handle bounce effect
        _DelayMS(2000);
      }

      if (notif & (EVT_FINGER_ADD | EVT_FINGER_DEL | EVT_FINGER_RST)) {
        // get driver value
        if (osMessageQueueGet(DriverQueueHandle, &driver, NULL, 0U) == osOK) {
          if (notif & EVT_FINGER_ADD)
            p = FINGER_Enroll(driver);
          else if (notif & EVT_FINGER_DEL)
            p = FINGER_DeleteID(driver);
          else if (notif & EVT_FINGER_RST)
            p = FINGER_EmptyDatabase();

          // handle response
          osThreadFlagsSet(CommandTaskHandle, p ? EVT_COMMAND_OK : EVT_COMMAND_ERROR);
        }
      }

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
  _osThreadFlagsWait(&notif, EVT_AUDIO_TASK_START, osFlagsWaitAll, osWaitForever);
  osThreadFlagsClear(EVT_AUDIO_TASK_STOP);

  /* Initialize Wave player (Codec, DMA, I2C) */
  AUDIO_Init(&hi2c1, &hi2s3);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.audio.wakeup = _GetTickMS() / 1000;

    // wait with timeout
    if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 1000)) {
      if (notif & EVT_AUDIO_TASK_STOP) {
        AUDIO_DeInit();
        _osThreadFlagsWait(&notif, EVT_AUDIO_TASK_START, osFlagsWaitAll, osWaitForever);
        AUDIO_Init(&hi2c1, &hi2s3);
      }

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
    AUDIO_OUT_SetVolume(VCU.SpeedToVolume());
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
  can_rx_t Rx;

  // wait until ManagerTask done
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

  /* Infinite loop */
  for (;;) {
    VCU.d.task.canRx.wakeup = _GetTickMS() / 1000;

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
          switch (CANBUS_ReadID(&(Rx.header)) >> 20) {
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
    VCU.d.task.canTx.wakeup = _GetTickMS() / 1000;
    lastWake = _GetTickMS();

    // send every 20m
    VCU.can.t.SwitchModeControl(&HBAR);

    // send every 500ms
    if (lastWake - last500ms > 500) {
      last500ms = _GetTickMS();

      VCU.can.t.MixedData(&(HBAR.runner));
    }

    // send every 1000ms
    if (lastWake - last1000ms > 1000) {
      last1000ms = _GetTickMS();

      VCU.can.t.Datetime(&(VCU.d.rtc.timestamp));
      VCU.can.t.SubTripData(&(HBAR.runner.mode.d.trip[0]));
    }

    // Handle CAN-Powered Nodes
    BMS.PowerOverCan(VCU.d.state.vehicle == VEHICLE_RUN);
    if (HMI2.PowerOverCan(VCU.d.state.vehicle >= VEHICLE_STANDBY))
      osThreadFlagsSet(Hmi2PowerTaskHandle, EVT_HMI2POWER_CHANGED);

    // Refresh CAN-Signaled Nodes
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
    VCU.d.task.hmi2Power.wakeup = _GetTickMS() / 1000;

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

  // wait until ManagerTask done
  osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

  // Initiate
  HBAR_ReadStates();
  VCU.d.gpio.power5v = GATE_ReadPower5v();
  VCU.d.gpio.knob = GATE_ReadKnobState();

  /* Infinite loop */
  for (;;) {
    VCU.d.task.gate.wakeup = _GetTickMS() / 1000;

    // wait forever
    if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 500)) {
      // handle bounce effect
      _DelayMS(50);
      //      osThreadFlagsClear(EVT_MASK);

      // Starter Button IRQ
      if (notif & EVT_GATE_STARTER_IRQ)
        VCU.d.gpio.starter = 1;

      // KNOB IRQ
      if (notif & EVT_GATE_KNOB_IRQ)
        VCU.d.gpio.knob = GATE_ReadKnobState();

      // BMS Power IRQ
      if (notif & EVT_GATE_REG_5V_IRQ)
        VCU.d.gpio.power5v = GATE_ReadPower5v();

      // Handle switch EXTI interrupt
      if (notif & EVT_GATE_HBAR) {
        HBAR_ReadStates();
        HBAR_TimerSelectSet();
        HBAR_RunSelectOrSet();
      }

    }

    // GATE Output Control
    HMI1.Power(VCU.d.state.vehicle >= VEHICLE_STANDBY);
    GATE_FanBMS(BMS.d.overheat);
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

  if (GPIO_Pin == EXT_REG_5V_IRQ_Pin)
    osThreadFlagsSet(GateTaskHandle, EVT_GATE_REG_5V_IRQ);

  if (GPIO_Pin == EXT_STARTER_IRQ_Pin)
    osThreadFlagsSet(GateTaskHandle, EVT_GATE_STARTER_IRQ);

  if (GPIO_Pin == EXT_KNOB_IRQ_Pin)
    osThreadFlagsSet(GateTaskHandle, EVT_GATE_KNOB_IRQ);

  if (GPIO_Pin == EXT_FINGER_IRQ_Pin)
    osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_PLACED);

  if (GPIO_Pin == INT_REMOTE_IRQ_Pin)
    RMT_IrqHandler();

  for (uint8_t i = 0; i < HBAR_K_MAX; i++)
    if (GPIO_Pin == HBAR.list[i].pin)
      osThreadFlagsSet(GateTaskHandle, EVT_GATE_HBAR);
}

static void CheckVehicleState(void) {
  static vehicle_state_t lastState = VEHICLE_UNKNOWN;
  vehicle_state_t initialState;

  do {
    initialState = VCU.d.state.vehicle;

    switch (VCU.d.state.vehicle) {
      case VEHICLE_LOST:
        if (lastState != VEHICLE_LOST) {
          lastState = VEHICLE_LOST;

          VCU.d.interval = RPT_INTERVAL_LOST;
        }

        if (VCU.d.gpio.power5v)
          VCU.d.state.vehicle += 2;
        break;

      case VEHICLE_BACKUP:
        if (lastState != VEHICLE_BACKUP) {
          lastState = VEHICLE_BACKUP;

          VCU.d.tick.independent = _GetTickMS();
          VCU.d.interval = RPT_INTERVAL_BACKUP;

          osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_TASK_STOP);
          osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_TASK_STOP);
          osThreadFlagsSet(GyroTaskHandle, EVT_GYRO_TASK_STOP);
        }

        if (_GetTickMS() - VCU.d.tick.independent > (VCU_ACTIVATE_LOST ) * 1000)
          VCU.d.state.vehicle--;
        else if (VCU.d.gpio.power5v)
          VCU.d.state.vehicle++;
        break;

      case VEHICLE_NORMAL:
        if (lastState != VEHICLE_NORMAL) {
          lastState = VEHICLE_NORMAL;
          VCU.d.interval = RPT_INTERVAL_NORMAL;
          VCU.d.state.override = VEHICLE_NORMAL;

          BAT_ReInit();

          osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_TASK_START);
          osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_TASK_START);
          osThreadFlagsSet(GyroTaskHandle, EVT_GYRO_TASK_START);
          osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_TASK_STOP);
        }

        if (!VCU.d.gpio.power5v)
          VCU.d.state.vehicle--;
        else if (VCU.d.gpio.knob
            && (!HMI1.d.state.unremote || VCU.d.state.override >= VEHICLE_STANDBY))
          VCU.d.state.vehicle++;
        break;

      case VEHICLE_STANDBY:
        if (lastState != VEHICLE_STANDBY) {
          lastState = VEHICLE_STANDBY;
          VCU.SetDriver(DRIVER_ID_NONE);

          osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_TASK_START);
        }

        if (!VCU.d.gpio.power5v || !VCU.d.gpio.knob)
          VCU.d.state.vehicle--;
        else if (!HMI1.d.state.unfinger || VCU.d.state.override >= VEHICLE_READY)
          VCU.d.state.vehicle++;
        break;

      case VEHICLE_READY:
        if (lastState != VEHICLE_READY) {
          lastState = VEHICLE_READY;
          VCU.d.gpio.starter = 0;
        }

        if (!VCU.d.gpio.power5v
            || !VCU.d.gpio.knob
            || (HMI1.d.state.unfinger && !VCU.d.state.override)
            || VCU.d.state.override == VEHICLE_STANDBY)
          VCU.d.state.vehicle--;
        else if (!VCU.d.state.error
            && (VCU.d.gpio.starter || VCU.d.state.override >= VEHICLE_RUN))
          VCU.d.state.vehicle++;
        break;

      case VEHICLE_RUN:
        if (lastState != VEHICLE_RUN) {
          lastState = VEHICLE_RUN;
          VCU.d.gpio.starter = 0;
        }

        if (!VCU.d.gpio.power5v
            || !VCU.d.gpio.knob
            || VCU.d.state.error
            || VCU.d.state.override == VEHICLE_READY)
          VCU.d.state.vehicle--;
        else if ((VCU.d.gpio.starter && VCU.d.speed == 0)
            || (HMI1.d.state.unfinger && VCU.d.state.override < VEHICLE_READY)
            || VCU.d.state.override == VEHICLE_STANDBY)
          VCU.d.state.vehicle -= 2;
        break;

      default:
        break;
    }
  } while (initialState != VCU.d.state.vehicle);
}

static void CheckTaskState(rtos_task_t *rtos) {
  rtos->manager.stack = osThreadGetStackSpace(ManagerTaskHandle);
  rtos->iot.stack = osThreadGetStackSpace(IotTaskHandle);
  rtos->reporter.stack = osThreadGetStackSpace(ReporterTaskHandle);
  rtos->command.stack = osThreadGetStackSpace(CommandTaskHandle);
  rtos->gps.stack = osThreadGetStackSpace(GpsTaskHandle);
  rtos->gyro.stack = osThreadGetStackSpace(GyroTaskHandle);
  rtos->remote.stack = osThreadGetStackSpace(RemoteTaskHandle);
  rtos->finger.stack = osThreadGetStackSpace(FingerTaskHandle);
  rtos->audio.stack = osThreadGetStackSpace(AudioTaskHandle);
  rtos->gate.stack = osThreadGetStackSpace(GateTaskHandle);
  rtos->canRx.stack = osThreadGetStackSpace(CanRxTaskHandle);
  rtos->canTx.stack = osThreadGetStackSpace(CanTxTaskHandle);
  rtos->hmi2Power.stack = osThreadGetStackSpace(Hmi2PowerTaskHandle);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
