/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "_database.h"
#include "_crc.h"
#include "_simcom.h"
#include "_gyro.h"
#include "_gps.h"
#include "_finger.h"
#include "_audio.h"
#include "_keyless.h"
#include "_reporter.h"
#include "_can.h"
#include "_rtc.h"
#include "_utils.h"
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
ADC_HandleTypeDef hadc1;

CAN_HandleTypeDef hcan1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

osThreadId IotTaskHandle;
osThreadId GyroTaskHandle;
osThreadId CommandTaskHandle;
osThreadId GpsTaskHandle;
osThreadId FingerTaskHandle;
osThreadId AudioTaskHandle;
osThreadId KeylessTaskHandle;
osThreadId ReporterTaskHandle;
osThreadId CanRxTaskHandle;
osThreadId SwitchTaskHandle;
osThreadId GeneralTaskHandle;
osThreadId CanTxTaskHandle;
osMessageQId VolumeQueueHandle;
osMutexId BeepMutexHandle;
osMutexId LogMutexHandle;
osMutexId CanTxMutexHandle;
osMutexId SimcomRecMutexHandle;
osMutexId FingerRecMutexHandle;
/* USER CODE BEGIN PV */
osMailQId GpsMailHandle;
osMailQId CommandMailHandle;
osMailQId ReportMailHandle;
osMailQId ResponseMailHandle;
extern db_t DB;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_CRC_Init(void);
static void MX_IWDG_Init(void);
void StartIotTask(const void *argument);
void StartGyroTask(const void *argument);
void StartCommandTask(const void *argument);
void StartGpsTask(const void *argument);
void StartFingerTask(const void *argument);
void StartAudioTask(const void *argument);
void StartKeylessTask(const void *argument);
void StartReporterTask(const void *argument);
void StartCanRxTask(const void *argument);
void StartSwitchTask(const void *argument);
void StartGeneralTask(const void *argument);
void StartCanTxTask(const void *argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
//  MX_CAN1_Init();
  MX_I2C3_Init();
  MX_USART2_UART_Init();
  MX_UART4_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_RTC_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
//  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  //  CANBUS_Init();
  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of BeepMutex */
  osMutexDef(BeepMutex);
  BeepMutexHandle = osMutexCreate(osMutex(BeepMutex));

  /* definition and creation of LogMutex */
  osMutexDef(LogMutex);
  LogMutexHandle = osMutexCreate(osMutex(LogMutex));

  /* definition and creation of CanTxMutex */
  osMutexDef(CanTxMutex);
  CanTxMutexHandle = osMutexCreate(osMutex(CanTxMutex));

  /* Create the recursive mutex(es) */
  /* definition and creation of SimcomRecMutex */
  osMutexDef(SimcomRecMutex);
  SimcomRecMutexHandle = osRecursiveMutexCreate(osMutex(SimcomRecMutex));

  /* definition and creation of FingerRecMutex */
  osMutexDef(FingerRecMutex);
  FingerRecMutexHandle = osRecursiveMutexCreate(osMutex(FingerRecMutex));

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
  /* definition and creation of VolumeQueue */
  osMessageQDef(VolumeQueue, 1, uint8_t);
  VolumeQueueHandle = osMessageCreate(osMessageQ(VolumeQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  osMailQDef(GpsMail, 1, gps_t);
  GpsMailHandle = osMailCreate(osMailQ(GpsMail), NULL);

  osMailQDef(CommandMail, 1, command_t);
  CommandMailHandle = osMailCreate(osMailQ(CommandMail), NULL);

  osMailQDef(ResponseMail, 1, response_t);
  ResponseMailHandle = osMailCreate(osMailQ(ResponseMail), NULL);

  osMailQDef(ReportMail, 100, report_t);
  ReportMailHandle = osMailCreate(osMailQ(ReportMail), NULL);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of IotTask */
  osThreadDef(IotTask, StartIotTask, osPriorityNormal, 0, 256);
  IotTaskHandle = osThreadCreate(osThread(IotTask), NULL);

  /* definition and creation of GyroTask */
  osThreadDef(GyroTask, StartGyroTask, osPriorityNormal, 0, 256);
//  GyroTaskHandle = osThreadCreate(osThread(GyroTask), NULL);

  /* definition and creation of CommandTask */
  osThreadDef(CommandTask, StartCommandTask, osPriorityAboveNormal, 0, 256);
//  CommandTaskHandle = osThreadCreate(osThread(CommandTask), NULL);

  /* definition and creation of GpsTask */
  osThreadDef(GpsTask, StartGpsTask, osPriorityNormal, 0, 256);
  GpsTaskHandle = osThreadCreate(osThread(GpsTask), NULL);

  /* definition and creation of FingerTask */
  osThreadDef(FingerTask, StartFingerTask, osPriorityNormal, 0, 256);
//  FingerTaskHandle = osThreadCreate(osThread(FingerTask), NULL);

  /* definition and creation of AudioTask */
  osThreadDef(AudioTask, StartAudioTask, osPriorityNormal, 0, 128);
//  AudioTaskHandle = osThreadCreate(osThread(AudioTask), NULL);

  /* definition and creation of KeylessTask */
  osThreadDef(KeylessTask, StartKeylessTask, osPriorityAboveNormal, 0, 256);
//  KeylessTaskHandle = osThreadCreate(osThread(KeylessTask), NULL);

  /* definition and creation of ReporterTask */
  osThreadDef(ReporterTask, StartReporterTask, osPriorityNormal, 0, 512);
  ReporterTaskHandle = osThreadCreate(osThread(ReporterTask), NULL);

  /* definition and creation of CanRxTask */
  osThreadDef(CanRxTask, StartCanRxTask, osPriorityRealtime, 0, 128);
//  CanRxTaskHandle = osThreadCreate(osThread(CanRxTask), NULL);

  /* definition and creation of SwitchTask */
  osThreadDef(SwitchTask, StartSwitchTask, osPriorityNormal, 0, 128);
//  SwitchTaskHandle = osThreadCreate(osThread(SwitchTask), NULL);

  /* definition and creation of GeneralTask */
  osThreadDef(GeneralTask, StartGeneralTask, osPriorityNormal, 0, 128);
  GeneralTaskHandle = osThreadCreate(osThread(GeneralTask), NULL);

  /* definition and creation of CanTxTask */
  osThreadDef(CanTxTask, StartCanTxTask, osPriorityHigh, 0, 128);
//  CanTxTaskHandle = osThreadCreate(osThread(CanTxTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // FIXME use tickless idle feature for low power mode
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE
      | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
      | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
      {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S | RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 256;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 5;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
      {
    Error_Handler();
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = { 0 };

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
      {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 102;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
 * @brief I2C3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
 * @brief I2S3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = { 0 };
  RTC_DateTypeDef sDate = { 0 };

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
   */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
      {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != RTC_ONE_TIME_RESET) {
    /* USER CODE END Check_RTC_BKUP */

    /** Initialize RTC and set the Time and Date
     */
    sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
        {
      Error_Handler();
    }
    sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x1;
    sDate.Year = 0x20;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
        {
      Error_Handler();
    }
    /* USER CODE BEGIN RTC_Init 2 */
  }

  /* USER CODE END RTC_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
 * @brief UART4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 57600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
      {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, INT_KEYLESS_CE_Pin | INT_NET_PWR_Pin | INT_GPS_PWR_Pin | EXT_FINGER_PWR_Pin
      | EXT_HMI1_PWR_Pin | EXT_HMI2_PWR_Pin | INT_AUDIO_PWR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EXT_FINGER_TOUCH_PWR_GPIO_Port, EXT_FINGER_TOUCH_PWR_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, EXT_SOLENOID_PWR_Pin | INT_NET_RST_Pin | INT_NET_DTR_Pin | INT_GYRO_PWR_Pin
      | INT_KEYLESS_PWR_Pin | EXT_KEYLESS_ALARM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, EXT_GPIO_OUT1_Pin | EXT_GPIO_OUT2_Pin | EXT_GPIO_OUT3_Pin | EXT_GPIO_OUT4_Pin
      | SYS_LED_Pin | EXT_HMI2_SHUTDOWN_Pin | EXT_HMI2_BRIGHTNESS_Pin | INT_AUDIO_RST_Pin
      | EXT_BMS_WAKEUP_Pin | EXT_BMS_FAN_Pin | EXT_REG_12V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(INT_KEYLESS_CSN_GPIO_Port, INT_KEYLESS_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : EXT_HBAR_SELECT_Pin EXT_HBAR_SET_Pin EXT_HBAR_REVERSE_Pin EXT_ABS_STATUS_Pin
   EXT_HBAR_SEIN_L_Pin EXT_HBAR_SEIN_R_Pin */
  GPIO_InitStruct.Pin = EXT_HBAR_SELECT_Pin | EXT_HBAR_SET_Pin | EXT_HBAR_REVERSE_Pin | EXT_ABS_STATUS_Pin
      | EXT_HBAR_SEIN_L_Pin | EXT_HBAR_SEIN_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : INT_KEYLESS_CE_Pin INT_GPS_PWR_Pin EXT_FINGER_TOUCH_PWR_Pin EXT_FINGER_PWR_Pin
   EXT_HMI1_PWR_Pin EXT_HMI2_PWR_Pin INT_AUDIO_PWR_Pin */
  GPIO_InitStruct.Pin = INT_KEYLESS_CE_Pin | INT_GPS_PWR_Pin | EXT_FINGER_TOUCH_PWR_Pin | EXT_FINGER_PWR_Pin
      | EXT_HMI1_PWR_Pin | EXT_HMI2_PWR_Pin | INT_AUDIO_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_NET_PWR_Pin */
  GPIO_InitStruct.Pin = INT_NET_PWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(INT_NET_PWR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EXT_SOLENOID_PWR_Pin INT_NET_RST_Pin INT_GYRO_PWR_Pin INT_KEYLESS_PWR_Pin
   EXT_KEYLESS_ALARM_Pin */
  GPIO_InitStruct.Pin = EXT_SOLENOID_PWR_Pin | INT_NET_RST_Pin | INT_GYRO_PWR_Pin | INT_KEYLESS_PWR_Pin
      | EXT_KEYLESS_ALARM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : EXT_FINGER_IRQ_Pin EXT_KNOB_IRQ_Pin EXT_HBAR_LAMP_Pin EXT_BMS_IRQ_Pin */
  GPIO_InitStruct.Pin = EXT_FINGER_IRQ_Pin | EXT_KNOB_IRQ_Pin | EXT_HBAR_LAMP_Pin | EXT_BMS_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : EXT_GPIO_IN1_Pin EXT_GPIO_IN2_Pin EXT_GPIO_IN3_Pin EXT_GPIO_IN4_Pin */
  GPIO_InitStruct.Pin = EXT_GPIO_IN1_Pin | EXT_GPIO_IN2_Pin | EXT_GPIO_IN3_Pin | EXT_GPIO_IN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_KEYLESS_IRQ_Pin */
  GPIO_InitStruct.Pin = INT_KEYLESS_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(INT_KEYLESS_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_NET_DTR_Pin */
  GPIO_InitStruct.Pin = INT_NET_DTR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(INT_NET_DTR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EXT_GPIO_OUT1_Pin EXT_GPIO_OUT2_Pin EXT_GPIO_OUT3_Pin EXT_GPIO_OUT4_Pin
   SYS_LED_Pin EXT_HMI2_SHUTDOWN_Pin EXT_HMI2_BRIGHTNESS_Pin EXT_BMS_WAKEUP_Pin
   EXT_BMS_FAN_Pin EXT_REG_12V_Pin */
  GPIO_InitStruct.Pin = EXT_GPIO_OUT1_Pin | EXT_GPIO_OUT2_Pin | EXT_GPIO_OUT3_Pin | EXT_GPIO_OUT4_Pin
      | SYS_LED_Pin | EXT_HMI2_SHUTDOWN_Pin | EXT_HMI2_BRIGHTNESS_Pin | EXT_BMS_WAKEUP_Pin
      | EXT_BMS_FAN_Pin | EXT_REG_12V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : EXT_GPIO_IN0_Pin */
  GPIO_InitStruct.Pin = EXT_GPIO_IN0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EXT_GPIO_IN0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_KEYLESS_CSN_Pin */
  GPIO_InitStruct.Pin = INT_KEYLESS_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(INT_KEYLESS_CSN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_AUDIO_RST_Pin */
  GPIO_InitStruct.Pin = INT_AUDIO_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(INT_AUDIO_RST_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint8_t i;

  if (osKernelRunning()) {
    // handle NRF24 IRQ
    //    if (GPIO_Pin == INT_KEYLESS_IRQ_Pin) {
    //      KEYLESS_IrqHandler();
    //    }
    //
    //    // handle Finger-print IRQ
    //    if (GPIO_Pin == EXT_FINGER_IRQ_Pin) {
    //      xTaskNotifyFromISR(
    //          FingerTaskHandle,
    //          EVENT_FINGER_PLACED,
    //          eSetBits,
    //          &xHigherPriorityTaskWoken);
    //    }
    //
    //    // handle Switches EXTI
    //    for (i = 0; i < DB.vcu.sw.count; i++) {
    //      if (GPIO_Pin == DB.vcu.sw.list[i].pin) {
    //        xTaskNotifyFromISR(
    //            SwitchTaskHandle,
    //            (uint32_t ) GPIO_Pin,
    //            eSetBits,
    //            &xHigherPriorityTaskWoken);
    //
    //        break;
    //      }
    //    }
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartIotTask */
/**
 * @brief  Function implementing the iotTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartIotTask */
void StartIotTask(const void *argument)
{
  /* USER CODE BEGIN 5 */
  osEvent evt;
  report_t *hReport = NULL, report;
  response_t *hResponse = NULL;
  uint8_t success, size, seq;

  // Start simcom module
  SIMCOM_DMA_Init();
  Simcom_Init(SIMCOM_POWER_UP);

  /* Infinite loop */
  size = sizeof(hReport->header.prefix) +
      sizeof(hReport->header.crc) +
      sizeof(hReport->header.size);

  for (;;) {
    _DebugTask("IoT");
    // Set default
    success = 1;

    osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);
    // response frame
    {
      // check response log
      evt = osMailGet(ResponseMailHandle, 0);
      // check is mail ready
      if (evt.status == osEventMail) {
        // copy the pointer
        hResponse = evt.value.p;

        seq = 0;
        while (seq++ < 2) {
          // Send to server
          success = Simcom_Upload((char*) hResponse, size + hResponse->header.size);
          // handle SIMCOM reply
          if (success) {
            // validate ACK
            if (Simcom_ReadACK(&(hResponse->header))) {
              // Release back
              osMailFree(ResponseMailHandle, hResponse);

              break;
            }
          }
        }

        // Release otherwise
        if (!success) {
          // Release back
          osMailFree(ResponseMailHandle, hResponse);
        }
      }
    }

    // report frame
    {
      // check report log
      if (hReport == NULL) {
        evt = osMailGet(ReportMailHandle, 0);
        // check is mail ready
        if (evt.status == osEventMail) {
          // copy the pointer
          hReport = evt.value.p;
          // copy value
          report = *hReport;
        }
      }

      if (hReport) {
        seq = 0;
        while (seq++ < 2) {
          // get current sending date-time
          report.data.req.rtc_send_datetime = RTC_Read();
          // recalculate the CRC
          report.header.crc = CRC_Calculate8(
              (uint8_t*) &(report.header.size),
              report.header.size + sizeof(report.header.size), 1);
          // Send to server
          success = Simcom_Upload((char*) &report, size + report.header.size);

          // handle SIMCOM reply
          if (success) {
            // validate ACK
            if (Simcom_ReadACK(&(report.header))) {
              // Release back
              osMailFree(ReportMailHandle, hReport);

              // check log again
              evt = osMailGet(ReportMailHandle, 0);
              // check is mail ready
              if (evt.status == osEventMail) {
                // copy the pointer
                hReport = evt.value.p;
                // copy value
                report = *hReport;
              } else {
                hReport = NULL;
              }

              // exit
              break;
            }
          }
        }
      }
    }
    osRecursiveMutexRelease(SimcomRecMutexHandle);

    // save the SIMCOM event
    Reporter_WriteEvent(REPORT_NETWORK_RESTART, !success);

    // handle sending error
    if (!success) {
      // restart module
      _LedDisco(1000);
      Simcom_Init(SIMCOM_RESTART);
    }
  }

  // Give other threads a shot
  osDelay(100);
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartGyroTask */
/**
 * @brief Function implementing the gyroTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGyroTask */
void StartGyroTask(const void *argument)
{
  /* USER CODE BEGIN StartGyroTask */
  TickType_t last_wake;
  mems_t mems_calibration;
  mems_decision_t mems_decision;

  /* MPU6050 Initialization*/
  GYRO_Init();
  // Set calibrator
  mems_calibration = GYRO_Average(NULL, 500);
  // Give success indicator
  AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 100);

  /* Infinite loop */
  last_wake = xTaskGetTickCount();
  for (;;) {
    _DebugTask("Gyro");
    // Read all accelerometer, gyroscope (average)
    mems_decision = GYRO_Decision(&mems_calibration, 25);

    // Check accelerometer, happens when impact detected
    if (mems_decision.crash) {
      xTaskNotify(ReporterTaskHandle, EVENT_REPORTER_CRASH, eSetBits);
    }

    // Check gyroscope, happens when fall detected
    if (mems_decision.fall) {
      xTaskNotify(ReporterTaskHandle, EVENT_REPORTER_FALL, eSetBits);
      AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 0);
    } else {
      xTaskNotify(ReporterTaskHandle, EVENT_REPORTER_FALL_FIXED, eSetBits);
      AUDIO_BeepStop();
    }

    // Report interval
    vTaskDelayUntil(&last_wake, tick5ms);
  }
  /* USER CODE END StartGyroTask */
}

/* USER CODE BEGIN Header_StartCommandTask */
/**
 * @brief Function implementing the commandTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommandTask */
void StartCommandTask(const void *argument)
{
  /* USER CODE BEGIN StartCommandTask */
  int p;
  osEvent evt;
  command_t *hCommand;
  response_t *hResponse;
  extern response_t RESPONSE;

  // reset response frame to default
  Reporter_Reset(FR_RESPONSE);

  /* Infinite loop */
  for (;;) {
    _DebugTask("Command");
    // get command in queue
    evt = osMailGet(CommandMailHandle, osWaitForever);

    if (evt.status == osEventMail) {
      hCommand = evt.value.p;

      // default command response
      RESPONSE.data.code = RESPONSE_STATUS_OK;
      strcpy(RESPONSE.data.message, "");

      // handle the command
      switch (hCommand->data.code) {
        case CMD_CODE_GEN:
          switch (hCommand->data.sub_code) {
            case CMD_GEN_INFO:
              sprintf(RESPONSE.data.message, "VCU v."VCU_FIRMWARE_VERSION", "VCU_VENDOR" @ 20%d", VCU_BUILD_YEAR);
              break;

            case CMD_GEN_LED:
              _LedWrite((uint8_t) hCommand->data.value);
              break;

            default:
              RESPONSE.data.code = RESPONSE_STATUS_INVALID;
              break;
          }
          break;

        case CMD_CODE_HMI2:
          switch (hCommand->data.sub_code) {
            case CMD_HMI2_SHUTDOWN:
              DB.hmi2.shutdown = (uint8_t) hCommand->data.value;
              break;

            default:
              RESPONSE.data.code = RESPONSE_STATUS_INVALID;
              break;
          }
          break;

        case CMD_CODE_REPORT:
          switch (hCommand->data.sub_code) {
            case CMD_REPORT_RTC:
              RTC_Write(hCommand->data.value, &(DB.vcu.rtc));
              break;

            case CMD_REPORT_ODOM:
              Reporter_SetOdometer((uint32_t) hCommand->data.value);
              break;

            case CMD_REPORT_UNITID:
              Reporter_SetUnitID((uint32_t) hCommand->data.value);
              break;

            default:
              RESPONSE.data.code = RESPONSE_STATUS_INVALID;
              break;
          }
          break;

        case CMD_CODE_AUDIO:
          switch (hCommand->data.sub_code) {
            case CMD_AUDIO_BEEP:
              xTaskNotify(AudioTaskHandle, EVENT_AUDIO_BEEP, eSetBits);
              break;

            case CMD_AUDIO_MUTE:
              xTaskNotify(
                  AudioTaskHandle,
                  (uint8_t) hCommand->data.value ? EVENT_AUDIO_MUTE_ON : EVENT_AUDIO_MUTE_OFF,
                  eSetBits);
              break;

            case CMD_AUDIO_VOL:
              osMessagePut(
                  VolumeQueueHandle,
                  (uint8_t) hCommand->data.value,
                  osWaitForever);
              break;

            default:
              RESPONSE.data.code = RESPONSE_STATUS_INVALID;
              break;
          }
          break;

        case CMD_CODE_FINGER:
          switch (hCommand->data.sub_code) {
            case CMD_FINGER_ADD:
              p = Finger_Enroll((uint8_t) hCommand->data.value);
              break;

            case CMD_FINGER_DEL:
              p = Finger_DeleteID((uint8_t) hCommand->data.value);
              break;

            case CMD_FINGER_RST:
              p = Finger_EmptyDatabase();
              break;

            default:
              RESPONSE.data.code = RESPONSE_STATUS_INVALID;
              break;
          }
          // handle response from finger-print module
          if (p != FINGERPRINT_OK) {
            RESPONSE.data.code = RESPONSE_STATUS_ERROR;
          }
          break;

        default:
          RESPONSE.data.code = RESPONSE_STATUS_INVALID;
          break;
      }
      // release back
      osMailFree(CommandMailHandle, hCommand);

      // Get current snapshot
      Reporter_Capture(FR_RESPONSE);

      // Allocate memory
      hResponse = osMailAlloc(ResponseMailHandle, osWaitForever);
      // Copy snapshot of current report
      *hResponse = RESPONSE;
      // Put report to log
      osMailPut(ResponseMailHandle, hResponse);
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
void StartGpsTask(const void *argument)
{
  /* USER CODE BEGIN StartGpsTask */
  TickType_t last_wake;
  gps_t *hGps;

  // Start GPS module
  UBLOX_DMA_Init();
  GPS_Init();

  /* Infinite loop */
  last_wake = xTaskGetTickCount();
  for (;;) {
    _DebugTask("GPS");
    // Allocate memory
    hGps = osMailAlloc(GpsMailHandle, osWaitForever);

    // get GPS info
    if (GPS_Process(hGps)) {
      osMailPut(GpsMailHandle, hGps);
    }

    // Report interval
    vTaskDelayUntil(&last_wake, tick1000ms);
  }
  /* USER CODE END StartGpsTask */
}

/* USER CODE BEGIN Header_StartFingerTask */
/**
 * @brief Function implementing the FingerTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartFingerTask */
void StartFingerTask(const void *argument)
{
  /* USER CODE BEGIN StartFingerTask */
  uint32_t notif_value;

  // Initialization
  FINGER_DMA_Init();
  Finger_Init();

  /* Infinite loop */
  for (;;) {
    _DebugTask("Finger");
    // check if user put finger
    xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &notif_value, portMAX_DELAY);

    // proceed event
    if (notif_value & EVENT_FINGER_PLACED) {
      if (Finger_AuthFast() > 0) {
        // indicator when finger is registered
        _LedWrite(1);
        osDelay(1000);
      }
    }
    _LedWrite(0);
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
void StartAudioTask(const void *argument)
{
  /* USER CODE BEGIN StartAudioTask */
  TickType_t last_wake;
  uint32_t notif_value;
  osEvent evt;

  /* Initialize Wave player (Codec, DMA, I2C) */
  AUDIO_Init();
  // Play wave loop forever, handover to DMA, so CPU is free
  AUDIO_Play();

  /* Infinite loop */
  last_wake = xTaskGetTickCount();
  for (;;) {
    _DebugTask("Audio");
    // do this if events occurred
    if (xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, 0) == pdTRUE) {
      // Beeping command
      if (notif_value & EVENT_AUDIO_BEEP) {
        // Beep
        AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
        osDelay(250);
        AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
      }
      // Mute command
      if (notif_value & EVENT_AUDIO_MUTE_ON) {
        AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
      }
      if (notif_value & EVENT_AUDIO_MUTE_OFF) {
        AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
      }
    }

    // check if get volume message
    evt = osMessageGet(VolumeQueueHandle, 0);
    // do this if message arrived
    if (evt.status == osEventMessage) {
      AUDIO_OUT_SetVolume((uint8_t) evt.value.v);
    }

    // Report interval
    vTaskDelayUntil(&last_wake, tick500ms);
  }
  /* USER CODE END StartAudioTask */
}

/* USER CODE BEGIN Header_StartKeylessTask */
/**
 * @brief Function implementing the KeylessTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartKeylessTask */
void StartKeylessTask(const void *argument)
{
  /* USER CODE BEGIN StartKeylessTask */
  uint8_t msg;
  uint32_t notif_value;

  // initialization
  KEYLESS_Init();

  /* Infinite loop */
  for (;;) {
    _DebugTask("Keyless");
    // check if has new can message
    xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, portMAX_DELAY);

    // proceed event
    if (notif_value & EVENT_KEYLESS_RX_IT) {
      msg = KEYLESS_ReadPayload();

      // indicator
      LOG_Str("NRF received packet, msg = ");
      LOG_Hex8(msg);
      LOG_Enter();

      // just fun indicator
      AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, (msg + 1) * 100);
      for (int i = 0; i < ((msg + 1) * 2); i++) {
        _LedToggle();
        osDelay(50);
      }
    }
  }
  /* USER CODE END StartKeylessTask */
}

/* USER CODE BEGIN Header_StartReporterTask */
/**
 * @brief Function implementing the ReporterTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReporterTask */
void StartReporterTask(const void *argument)
{
  /* USER CODE BEGIN StartReporterTask */
  extern report_t REPORT;
  TickType_t last_wake, last_wake_full = 0;
  osEvent evt;
  frame_type frame;
  uint8_t net_restart_state;
  uint32_t notif_value;
  report_t *hReport;
  gps_t *hGps;

  // FIXME: create master thread
  // ONE-TIME configurations:
  if (EEPROM_Init()) {
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != RTC_ONE_TIME_RESET) {
      // reporter configuration
      Reporter_SetUnitID(REPORT_UNITID);
      Reporter_SetOdometer(0);
      // simcom configuration

      // re-write backup register
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_ONE_TIME_RESET);
    }
  }

  // reset report frame to default
  Reporter_Reset(FR_FULL);

  /* Infinite loop */
  last_wake = xTaskGetTickCount();
  for (;;) {
    _DebugTask("Reporter");
    // preserve simcom reboot from events group
    net_restart_state = Reporter_ReadEvent(REPORT_NETWORK_RESTART);
    // reset all events group
    Reporter_SetEvents(0);
    // re-write the simcom reboot events
    Reporter_WriteEvent(REPORT_NETWORK_RESTART, net_restart_state);

    // do this if events occurred
    if (xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, 0) == pdTRUE) {
      // check & set every event
      if (notif_value & EVENT_REPORTER_CRASH) {
        Reporter_WriteEvent(REPORT_BIKE_CRASHED, 1);
      }
      if ((notif_value & EVENT_REPORTER_FALL) && !(notif_value & EVENT_REPORTER_FALL_FIXED)) {
        Reporter_WriteEvent(REPORT_BIKE_FALLING, 1);
      }
    }

    // get processed GPS data
    evt = osMailGet(GpsMailHandle, 0);
    // break-down RAW NMEA data from GPS module
    if (evt.status == osEventMail) {
      hGps = evt.value.p;
      // set GPS data
      Reporter_SetGPS(hGps);
      // set speed value (based on GPS)
      Reporter_SetSpeed(hGps);
      // Release back
      osMailFree(GpsMailHandle, hGps);
    }

    // decide full/simple frame time
    if ((last_wake - last_wake_full) >= tickDelayFull) {
      // capture full frame wake time
      last_wake_full = last_wake;
      // full frame
      frame = FR_FULL;
    } else {
      // simple frame
      frame = FR_SIMPLE;
    }

    // Get current snapshot
    Reporter_Capture(frame);

    // Get log space
    do {
      // Allocate memory
      hReport = osMailAlloc(ReportMailHandle, osWaitForever);
      // Handle full log
      if (hReport == NULL) {
        // get first log
        evt = osMailGet(ReportMailHandle, 0);
        if (evt.status == osEventMail) {
          // remove it
          osMailFree(ReportMailHandle, evt.value.p);
        }
      }
    } while (hReport == NULL);
    // Copy snapshot of current report
    *hReport = REPORT;
    // Put report to log
    osMailPut(ReportMailHandle, hReport);

    // Report interval in second (based on lowest interval, the simple frame)
    vTaskDelayUntil(&last_wake, tickDelaySimple);
  }
  /* USER CODE END StartReporterTask */
}

/* USER CODE BEGIN Header_StartCanRxTask */
/**
 * @brief Function implementing the CanRxTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanRxTask */
void StartCanRxTask(const void *argument)
{
  /* USER CODE BEGIN StartCanRxTask */
  uint32_t notif_value;

  /* Infinite loop */
  for (;;) {
    _DebugTask("CanRx");
    // check if has new can message
    xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, portMAX_DELAY);

    // proceed event
    if (notif_value & EVENT_CAN_RX_IT) {
      // handle message
      switch (CANBUS_ReadID()) {
        case CAN_ADDR_MCU_DUMMY:
          CAN_MCU_Dummy_Read(&(DB.vcu.speed));
          // set volume by speed
          osMessagePut(VolumeQueueHandle, DB.vcu.speed, osWaitForever);
          break;
        default:
          break;
      }
    }

  }
  /* USER CODE END StartCanRxTask */
}

/* USER CODE BEGIN Header_StartSwitchTask */
/**
 * @brief Function implementing the SwitchTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartSwitchTask */
void StartSwitchTask(const void *argument)
{
  /* USER CODE BEGIN StartSwitchTask */
  uint8_t i, iModeDrive;
  uint32_t notif_value;

  // Read all EXTI state
  for (i = 0; i < DB.vcu.sw.count; i++) {
    DB.vcu.sw.list[i].state = HAL_GPIO_ReadPin(DB.vcu.sw.list[i].port, DB.vcu.sw.list[i].pin);
  }

  // Handle Reverse mode on init
  if (DB.vcu.sw.list[SW_K_REVERSE].state) {
    // save previous Drive Mode state
    if (DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] != SW_M_DRIVE_R) {
      iModeDrive = DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE];
    }
    // force state
    DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] = SW_M_DRIVE_R;
    // hazard on
    DB.vcu.sw.list[SW_K_SEIN_LEFT].state = 1;
    DB.vcu.sw.list[SW_K_SEIN_RIGHT].state = 1;
  }

  /* Infinite loop */
  for (;;) {
    _DebugTask("Switch");

    xTaskNotifyStateClear(NULL);
    xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, portMAX_DELAY);
    // handle bounce effect
    osDelay(50);

    // Read all (to handle multiple switch change at the same time)
    for (i = 0; i < DB.vcu.sw.count; i++) {
      DB.vcu.sw.list[i].state = HAL_GPIO_ReadPin(DB.vcu.sw.list[i].port, DB.vcu.sw.list[i].pin);

      // handle select & set: timer
      if (i == SW_K_SELECT || i == SW_K_SET) {
        // reset SET timer
        DB.vcu.sw.timer[i].time = 0;

        // next job
        if (DB.vcu.sw.list[i].state) {
          if (i == SW_K_SELECT || (i == SW_K_SET && DB.vcu.sw.runner.listening)) {
            // start timer if not running
            if (!DB.vcu.sw.timer[i].running) {
              // set flag
              DB.vcu.sw.timer[i].running = 1;
              // start timer for SET
              DB.vcu.sw.timer[i].start = osKernelSysTick();
            }
          }
          // reverse it
          DB.vcu.sw.list[i].state = 0;
        } else {
          // stop timer if running
          if (DB.vcu.sw.timer[i].running) {
            // set flag
            DB.vcu.sw.timer[i].running = 0;
            // stop SET
            DB.vcu.sw.timer[i].time = (uint8_t) ((osKernelSysTick()
                - DB.vcu.sw.timer[i].start) / tick1000ms);
            // reverse it
            DB.vcu.sw.list[i].state = 1;
          }
        }
      }
    }

    // Only handle Select & Set when in non-reverse mode
    if (DB.vcu.sw.list[SW_K_REVERSE].state) {
      // save previous Drive Mode state
      if (DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] != SW_M_DRIVE_R) {
        iModeDrive = DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE];
      }
      // force state
      DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] = SW_M_DRIVE_R;
      // hazard on
      DB.vcu.sw.list[SW_K_SEIN_LEFT].state = 1;
      DB.vcu.sw.list[SW_K_SEIN_RIGHT].state = 1;
    } else {
      // restore previous Drive Mode
      if (DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R) {
        DB.vcu.sw.runner.mode.sub.val[SW_M_DRIVE] = iModeDrive;
      }

      // handle Select & Set
      if (DB.vcu.sw.list[SW_K_SELECT].state || DB.vcu.sw.list[SW_K_SET].state) {
        // handle select key
        if (DB.vcu.sw.list[SW_K_SELECT].state) {
          if (DB.vcu.sw.runner.listening) {
            // change mode position
            if (DB.vcu.sw.runner.mode.val == SW_M_MAX) {
              DB.vcu.sw.runner.mode.val = 0;
            } else {
              DB.vcu.sw.runner.mode.val++;
            }
          }
          // Listening on option
          DB.vcu.sw.runner.listening = 1;

        } else if (DB.vcu.sw.list[SW_K_SET].state) {
          // handle set key
          if (DB.vcu.sw.runner.listening
              || (DB.vcu.sw.timer[SW_K_SET].time >= 3 && DB.vcu.sw.runner.mode.val == SW_M_TRIP)) {
            // handle reset only if push more than n sec, and in trip mode
            if (!DB.vcu.sw.runner.listening) {
              // reset value
              DB.vcu.sw.runner.mode.sub.trip[DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val]] = 0;
            } else {
              // if less than n sec
              if (DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val]
                  == DB.vcu.sw.runner.mode.sub.max[DB.vcu.sw.runner.mode.val]) {
                DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val] = 0;
              } else {
                DB.vcu.sw.runner.mode.sub.val[DB.vcu.sw.runner.mode.val]++;
              }
            }
          }
        }
      }
    }
  }
  /* USER CODE END StartSwitchTask */
}

/* USER CODE BEGIN Header_StartGeneralTask */
/**
 * @brief Function implementing the GeneralTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGeneralTask */
void StartGeneralTask(const void *argument)
{
  /* USER CODE BEGIN StartGeneralTask */
  TickType_t last_wake;
  timestamp_t timestampCarrier;

  /* Infinite loop */
  last_wake = xTaskGetTickCount();

  for (;;) {
    _DebugTask("General");
    // Retrieve network signal quality
    Simcom_ReadSignal(&(DB.vcu.signal));

    //    // Retrieve RTC time
    //    RTC_ReadRaw(&(DB.vcu.rtc.timestamp));
    //
    //    // Check calibration by cellular network
    //    if (_TimeNeedCalibration(DB.vcu.rtc)) {
    //      // get carrier timestamp
    //      if (Simcom_ReadTime(&timestampCarrier)) {
    //        // calibrate the RTC
    //        RTC_WriteRaw(&timestampCarrier, &(DB.vcu.rtc));
    //      }
    //    }
    //
    //    // Dummy data generator
    //    _DummyGenerator(&DB);

    // Toggling LED
    _LedToggle();

    // Periodic interval
    vTaskDelayUntil(&last_wake, tick500ms);
  }
  /* USER CODE END StartGeneralTask */
}

/* USER CODE BEGIN Header_StartCanTxTask */
/**
 * @brief Function implementing the CanTxTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCanTxTask */
void StartCanTxTask(const void *argument)
{
  /* USER CODE BEGIN StartCanTxTask */
  TickType_t last_wake;

  /* Infinite loop */
  last_wake = xTaskGetTickCount();
  for (;;) {
    _DebugTask("CanTx");
    // Send CAN data
    CAN_VCU_Switch(&DB);
    CAN_VCU_RTC(&(DB.vcu.rtc.timestamp));
    CAN_VCU_Select_Set(&(DB.vcu.sw.runner));
    CAN_VCU_Trip_Mode(&(DB.vcu.sw.runner.mode.sub.trip[0]));

    // Feed the dog (duration x seconds)
    HAL_IWDG_Refresh(&hiwdg);

    // Periodic interval
    vTaskDelayUntil(&last_wake, tick250ms);
  }
  /* USER CODE END StartCanTxTask */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  _Error("Error Handler fired.");

  while (1)
    ;
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
