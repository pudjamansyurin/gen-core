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
#include "_handlebar.h"
#include "_dma_battery.h"
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
DMA_HandleTypeDef hdma_adc1;

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
		.stack_size = 352 * 4
};
/* Definitions for ReporterTask */
osThreadId_t ReporterTaskHandle;
const osThreadAttr_t ReporterTask_attributes = {
		.name = "ReporterTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 352 * 4
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
		.stack_size = 224 * 4
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
		.stack_size = 224 * 4
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
/* Definitions for RTCMutex */
osMutexId_t RTCMutexHandle;
const osMutexAttr_t RTCMutex_attributes = {
		.name = "RTCMutex"
};
/* Definitions for CRCMutex */
osMutexId_t CRCMutexHandle;
const osMutexAttr_t CRCMutex_attributes = {
		.name = "CRCMutex"
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
/* USER CODE BEGIN PV */
osEventFlagsId_t GlobalEventHandle;

extern db_t DB;
extern sw_t SW;
extern sim_t SIM;
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
	MX_CAN1_Init();
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
	MX_IWDG_Init();
	/* USER CODE BEGIN 2 */
	CANBUS_Init();
	BAT_DMA_Init();
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();
	/* Create the mutex(es) */
	/* creation of AudioMutex */
	AudioMutexHandle = osMutexNew(&AudioMutex_attributes);

	/* creation of LogMutex */
	LogMutexHandle = osMutexNew(&LogMutex_attributes);

	/* creation of CanTxMutex */
	CanTxMutexHandle = osMutexNew(&CanTxMutex_attributes);

	/* creation of EepromMutex */
	EepromMutexHandle = osMutexNew(&EepromMutex_attributes);

	/* creation of RTCMutex */
	RTCMutexHandle = osMutexNew(&RTCMutex_attributes);

	/* creation of CRCMutex */
	CRCMutexHandle = osMutexNew(&CRCMutex_attributes);

	/* Create the recursive mutex(es) */
	/* creation of SimcomRecMutex */
	SimcomRecMutexHandle = osMutexNew(&SimcomRecMutex_attributes);

	/* creation of FingerRecMutex */
	FingerRecMutexHandle = osMutexNew(&FingerRecMutex_attributes);

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

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	HAL_Delay(1000);
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
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
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
			{
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_9;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
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
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != RTC_RESET) {
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
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_RESET);
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
	/* DMA2_Stream0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
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
	 EXT_KNOB_IRQ_Pin EXT_HBAR_LAMP_Pin EXT_BMS_IRQ_Pin EXT_HBAR_SEIN_L_Pin
	 EXT_HBAR_SEIN_R_Pin */
	GPIO_InitStruct.Pin = EXT_HBAR_SELECT_Pin | EXT_HBAR_SET_Pin | EXT_HBAR_REVERSE_Pin | EXT_ABS_STATUS_Pin
			| EXT_KNOB_IRQ_Pin | EXT_HBAR_LAMP_Pin | EXT_BMS_IRQ_Pin | EXT_HBAR_SEIN_L_Pin
			| EXT_HBAR_SEIN_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PE4 */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : INT_KEYLESS_CE_Pin INT_NET_PWR_Pin INT_GPS_PWR_Pin EXT_FINGER_TOUCH_PWR_Pin
	 EXT_FINGER_PWR_Pin EXT_HMI1_PWR_Pin EXT_HMI2_PWR_Pin INT_AUDIO_PWR_Pin */
	GPIO_InitStruct.Pin = INT_KEYLESS_CE_Pin | INT_NET_PWR_Pin | INT_GPS_PWR_Pin | EXT_FINGER_TOUCH_PWR_Pin
			| EXT_FINGER_PWR_Pin | EXT_HMI1_PWR_Pin | EXT_HMI2_PWR_Pin | INT_AUDIO_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : EXT_SOLENOID_PWR_Pin INT_NET_RST_Pin INT_NET_DTR_Pin INT_GYRO_PWR_Pin
	 INT_KEYLESS_PWR_Pin EXT_KEYLESS_ALARM_Pin */
	GPIO_InitStruct.Pin = EXT_SOLENOID_PWR_Pin | INT_NET_RST_Pin | INT_NET_DTR_Pin | INT_GYRO_PWR_Pin
			| INT_KEYLESS_PWR_Pin | EXT_KEYLESS_ALARM_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : EXT_FINGER_IRQ_Pin */
	GPIO_InitStruct.Pin = EXT_FINGER_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(EXT_FINGER_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : EXT_GPIO_IN1_Pin EXT_GPIO_IN2_Pin EXT_GPIO_IN3_Pin EXT_GPIO_IN4_Pin */
	GPIO_InitStruct.Pin = EXT_GPIO_IN1_Pin | EXT_GPIO_IN2_Pin | EXT_GPIO_IN3_Pin | EXT_GPIO_IN4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
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
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
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
	if (osKernelGetState() == osKernelRunning) {
		// handle BMS_IRQ (is 5v exist?)
		if (GPIO_Pin == EXT_BMS_IRQ_Pin) {
			osThreadFlagsSet(ManagerTaskHandle, EVT_MANAGER_BMS_IRQ);
		}
		// handle KNOB IRQ (Power control for HMI1 & HMI2)
		if (GPIO_Pin == EXT_KNOB_IRQ_Pin) {
			osThreadFlagsSet(ManagerTaskHandle, EVT_MANAGER_KNOB_IRQ);
		}
		//		// handle Finger IRQ
		//		if (GPIO_Pin == EXT_FINGER_IRQ_Pin) {
		//			osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_PLACED);
		//		}
		// handle NRF24 IRQ
		if (GPIO_Pin == INT_KEYLESS_IRQ_Pin) {
			KEYLESS_IrqHandler();
		}

		// handle Switches EXTI
		for (uint8_t i = 0; i < SW_TOTAL_LIST; i++) {
			if (GPIO_Pin == SW.list[i].pin) {
				osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_TRIGGERED);

				break;
			}
		}
	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartManagerTask */
/**
 * @brief  Function implementing the ManagerTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartManagerTask */
void StartManagerTask(void *argument)
{
	/* USER CODE BEGIN 5 */
	TickType_t lastWake, lastDebug;
	uint32_t notif;
	uint8_t thCount = osThreadGetCount();
	osThreadId_t threads[thCount];

	// NOTE: This task get executed first!
	DB_Init();

	// EEPROM: Initialization & Load
	EEPROM_ResetOrLoad();

	// Check GPIOs state
	DB_VCU_CheckBMSPresence();
	DB.vcu.knob = HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);

	// Threads management:
	osThreadSuspend(GyroTaskHandle);
	osThreadSuspend(AudioTaskHandle);
	osThreadSuspend(FingerTaskHandle);

	// Release threads
	osEventFlagsSet(GlobalEventHandle, EVENT_READY);

	// Get list of active threads
	osThreadEnumerate(threads, thCount);

	/* Infinite loop */
	lastDebug = osKernelGetTickCount();
	for (;;) {
		_DebugTask("Manager");
		lastWake = osKernelGetTickCount();

		// Handle GPIO interrupt
		notif = osThreadFlagsGet();
		if (notif) {
			// handle bounce effect
			osDelay(50);
			osThreadFlagsClear(notif);

			// BMS Power IRQ
			if (notif & EVT_MANAGER_BMS_IRQ) {
				// get current state
				DB_VCU_CheckBMSPresence();
			}
			// KNOB IRQ
			if (notif & EVT_MANAGER_KNOB_IRQ) {
				// get current state
				DB.vcu.knob = HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);
			}
		}

		// Dummy data generator
		_DummyGenerator(&DB, &SW);

		// Check is Daylight
		DB.hmi1.status.daylight = _TimeCheckDaylight(DB.vcu.rtc.timestamp);

		// Feed the dog
		HAL_IWDG_Refresh(&hiwdg);

		// Battery Monitor
		LOG_Str("Battery:Voltage = ");
		LOG_Int(DB.vcu.bat_voltage);
		LOG_StrLn(" mV");

		// Thread's Stack Monitor
		if ((osKernelGetTickCount() - lastDebug) >= pdMS_TO_TICKS(10000)) {
			_DebugStackSpace(threads, thCount);

			lastDebug = osKernelGetTickCount();
		}

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(1000));
	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartIotTask */
/**
 * @brief  Function implementing the iotTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartIotTask */
void StartIotTask(void *argument)
{
	/* USER CODE BEGIN StartIotTask */
	osStatus_t status;
	SIMCOM_RESULT p;
	report_t report;
	response_t response;
	timestamp_t timestamp;
	uint8_t retry, nack;
	uint8_t pending[2] = { 0 };
	osMessageQueueId_t *pQueue;
	header_t *pHeader;
	void *pPayload;
	const uint8_t size = sizeof(report.header.prefix) +
			sizeof(report.header.crc) +
			sizeof(report.header.size);

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// Start simcom module
	SIMCOM_DMA_Init();
	Simcom_SetState(SIM_STATE_CONFIGURED);

	/* Infinite loop */
	for (;;) {
		_DebugTask("IoT");

		// Upload Report & Response Payload
		for (uint8_t type = 0; type <= PAYLOAD_MAX; type++) {
			// decide the payload
			if (type == PAYLOAD_REPORT) {
				pQueue = &ReportQueueHandle;
				pPayload = &report;
			} else {
				pQueue = &ResponseQueueHandle;
				pPayload = &response;
			}
			pHeader = (header_t*) pPayload;

			// check report log
			if (!pending[type]) {
				status = osMessageQueueGet(*pQueue, pPayload, NULL, 0);
				// check is mail ready
				if (status == osOK) {
					pending[type] = 1;
				}
			}

			// check is payload ready
			if (pending[type]) {
				retry = 1;
				nack = 1;

				do {
					if (type == PAYLOAD_REPORT) {
						// Re-calculate CRC & Sending Time
						Report_ReCalculate((report_t*) pPayload);
					}

					// Send to server
					Simcom_SetState(SIM_STATE_SERVER_ON);
					p = Simcom_Upload(pPayload, size + pHeader->size, &retry);

					// Handle looping NACK
					if (p == SIM_RESULT_NACK) {
						if (nack++ >= SIMCOM_MAX_UPLOAD_RETRY) {
							// Probably  CRC not valid, cancel but force as success
							p = SIM_RESULT_OK;
						}
					}

					// Release back
					if (p == SIM_RESULT_OK) {
						EEPROM_SequentialID(EE_CMD_W, pHeader->seq_id, type);
						pending[type] = 0;

						break;
					}

					// delay
					osDelay(500);
				} while (p != SIM_RESULT_OK && retry <= SIMCOM_MAX_UPLOAD_RETRY);
			}
		}

		// ================= SIMCOM Related Routines ================
		Simcom_SetState(SIM_STATE_READY);
		// Retrieve network signal quality
		SIM_SignalQuality(&(DB.vcu.signal_percent));
		// Check calibration by cellular network
		if (_TimeNeedCalibration(&DB)) {
			// get carrier timestamp
			if (SIM_Clock(&timestamp)) {
				// calibrate the RTC
				RTC_WriteRaw(&timestamp, &(DB.vcu.rtc));
			}
		}

		// scan ADC while simcom at rest
		osDelay(1000);
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
	TickType_t lastWake, lastFull = 0;
	report_t report, tmp;
	osStatus_t status;
	FRAME_TYPE frame;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// Initialize
	Report_Init(FR_SIMPLE, &report);

	/* Infinite loop */
	for (;;) {
		_DebugTask("Reporter");
		lastWake = osKernelGetTickCount();

		// decide full/simple frame time
		if (!DB.vcu.independent) {
			// BMS plugged
			if ((lastWake - lastFull) >= pdMS_TO_TICKS(RPT_INTERVAL_FULL*1000)) {
				// capture full frame wake time
				lastFull = lastWake;
				// full frame
				frame = FR_FULL;
			} else {
				// simple frame
				frame = FR_SIMPLE;
			}
		} else {
			// BMS un-plugged
			frame = FR_FULL;
		}

		// Get current snapshot
		Report_Capture(frame, &report);

		do {
			// Put report to log
			status = osMessageQueuePut(ReportQueueHandle, &report, 0U, 0U);
			// already full, remove oldest
			if (status == osErrorResource) {
				osMessageQueueGet(ReportQueueHandle, &tmp, NULL, 0U);
			}
		} while (status != osOK);

		// reset some events group
		DB_SetEvent(EV_VCU_NETWORK_RESTART, 0);

		// Report interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(DB.vcu.interval * 1000));
	}
	/* USER CODE END StartReporterTask */
}

/* USER CODE BEGIN Header_StartCommandTask */
/**
 * @brief Function implementing the commandTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommandTask */
void StartCommandTask(void *argument)
{
	/* USER CODE BEGIN StartCommandTask */
	response_t response;
	osStatus_t status;
	command_t command;
	int p;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// Initialize
	Response_Init(&response);

	/* Infinite loop */
	for (;;) {
		_DebugTask("Command");
		// get command in queue
		status = osMessageQueueGet(CommandQueueHandle, &command, NULL, osWaitForever);

		if (status == osOK) {
			// debug
			LOG_Str("\nCommand:Payload [");
			LOG_Int(command.data.code);
			LOG_Str("-");
			LOG_Int(command.data.sub_code);
			LOG_Str("] = ");
			LOG_BufHex((char*) &(command.data.value), sizeof(command.data.value));
			LOG_Enter();

			// default command response
			response.data.code = RESPONSE_STATUS_OK;
			strcpy(response.data.message, "");

			// handle the command
			switch (command.data.code) {
				case CMD_CODE_GEN:
					switch (command.data.sub_code) {
						case CMD_GEN_INFO:
							sprintf(response.data.message, "VCU v."VCU_FIRMWARE_VERSION", "VCU_VENDOR" @ 20%d", VCU_BUILD_YEAR);
							break;

						case CMD_GEN_LED:
							_LedWrite((uint8_t) command.data.value);
							break;

						case CMD_GEN_KNOB:
							DB.vcu.knob = (uint8_t) command.data.value;
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_REPORT:
					switch (command.data.sub_code) {
						case CMD_REPORT_RTC:
							RTC_Write(command.data.value, &(DB.vcu.rtc));
							break;

						case CMD_REPORT_ODOM:
							EEPROM_Odometer(EE_CMD_W, (uint32_t) command.data.value);
							break;

						case CMD_REPORT_UNITID:
							EEPROM_UnitID(EE_CMD_W, (uint32_t) command.data.value);
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_AUDIO:
					switch (command.data.sub_code) {
						case CMD_AUDIO_BEEP:
							osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP);
							break;

						case CMD_AUDIO_MUTE:
							osThreadFlagsSet(
									AudioTaskHandle,
									(uint8_t) command.data.value ? EVT_AUDIO_MUTE_ON : EVT_AUDIO_MUTE_OFF);
							break;

						case CMD_AUDIO_VOL:
							DB.vcu.volume = command.data.value;
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_FINGER:
					switch (command.data.sub_code) {
						case CMD_FINGER_ADD:
							p = Finger_Enroll((uint8_t) command.data.value);
							break;

						case CMD_FINGER_DEL:
							p = Finger_DeleteID((uint8_t) command.data.value);
							break;

						case CMD_FINGER_RST:
							p = Finger_EmptyDatabase();
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					// handle response from finger-print module
					if (p != FINGERPRINT_OK) {
						response.data.code = RESPONSE_STATUS_ERROR;
					}
					break;

				default:
					response.data.code = RESPONSE_STATUS_INVALID;
					break;
			}

			// Get current snapshot
			Response_Capture(&response);
			// Send to Queue
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
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// Initialize
	UBLOX_DMA_Init();
	GPS_Init();

	/* Infinite loop */
	for (;;) {
		_DebugTask("GPS");
		lastWake = osKernelGetTickCount();

		GPS_Capture();
		// FIXME: Dummy odometer (based on GPS)
		GPS_CalculateOdometer();

		// debug
		//    LOG_StrLn("GPS:Buffer = ");
		//    LOG_Buf(UBLOX_UART_RX, strlen(UBLOX_UART_RX));
		//    LOG_Enter();

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(GPS_INTERVAL_MS));
	}
	/* USER CODE END StartGpsTask */
}

/* USER CODE BEGIN Header_StartGyroTask */
/**
 * @brief Function implementing the gyroTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGyroTask */
void StartGyroTask(void *argument)
{
	/* USER CODE BEGIN StartGyroTask */
	TickType_t lastWake;
	mems_t mems_calibration;
	mems_decision_t mems_decision;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	/* MPU6050 Initialization*/
	GYRO_Init();

	// Set calibrator
	mems_calibration = GYRO_Average(NULL, 500);
	LOG_StrLn("Gyro:Calibrated");

	/* Infinite loop */
	for (;;) {
		_DebugTask("Gyro");
		lastWake = osKernelGetTickCount();

		// Read all accelerometer, gyroscope (average)
		mems_decision = GYRO_Decision(&mems_calibration, 25);

		// Check accelerometer, happens when impact detected
		DB_SetEvent(EV_VCU_BIKE_CRASHED, mems_decision.crash);

		// Check gyroscope, happens when fall detected
		if (mems_decision.fall) {
			osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_START);
			DB_SetEvent(EV_VCU_BIKE_FALLING, 1);
			_LedDisco(1000);
		} else {
			osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_STOP);
			DB_SetEvent(EV_VCU_BIKE_FALLING, 0);
		}

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(100));
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
	uint8_t msg;
	uint32_t notif;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// initialization
	KEYLESS_Init();

	/* Infinite loop */
	for (;;) {
		_DebugTask("Keyless");

		// check if has new can message
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(100));
		{
			// proceed event
			if (notif & EVT_KEYLESS_RX_IT) {
				msg = KEYLESS_ReadPayload();

				// record heartbeat
				DB.vcu.tick.keyless = osKernelGetTickCount();

				// indicator
				LOG_Str("NRF received packet, msg = ");
				LOG_Hex8(msg);
				LOG_Enter();

				// just fun indicator
				// FIXME: AUDIO should be handled by AudioTask
				//        AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, (msg + 1) * 100);
				for (int i = 0; i < ((msg + 1) * 2); i++) {
					_LedToggle();
					osDelay(50);
				}
			}
		}

		// update heartbeat
		if ((osKernelGetTickCount() - DB.vcu.tick.keyless) < pdMS_TO_TICKS(5000)) {
			DB.hmi1.status.keyless = 1;
		} else {
			DB.hmi1.status.keyless = 0;
		}
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

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// Initialization
	FINGER_DMA_Init();
	Finger_Init();

	/* Infinite loop */
	for (;;) {
		_DebugTask("Finger");

		{
			// check if user put finger
			notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(100));

			// proceed event
			if (notif & EVT_FINGER_PLACED) {
				if (Finger_AuthFast() > 0) {
					// indicator when finger is registered
					_LedWrite(1);
					osDelay(1000);
					_LedWrite(0);
				}
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
	TickType_t lastWake;
	uint32_t notif;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	/* Initialize Wave player (Codec, DMA, I2C) */
	AUDIO_Init();
	// Play wave loop forever, handover to DMA, so CPU is free
	AUDIO_Play();

	/* Infinite loop */
	for (;;) {
		_DebugTask("Audio");
		lastWake = osKernelGetTickCount();

		// do this if events occurred
		notif = osThreadFlagsGet();
		if (notif) {
			osThreadFlagsClear(notif);

			// Beep command
			if (notif & EVT_AUDIO_BEEP) {
				// Beep
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
				osDelay(250);
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 250);
			}
			// Long-Beep Command
			if (notif & EVT_AUDIO_BEEP_START) {
				AUDIO_BeepPlay(BEEP_FREQ_2000_HZ, 0);
			}
			if (notif & EVT_AUDIO_BEEP_STOP) {
				AUDIO_BeepStop();
			}
			// Mute command
			if (notif & EVT_AUDIO_MUTE_ON) {
				AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
			}
			if (notif & EVT_AUDIO_MUTE_OFF) {
				AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
			}
		}

		// update volume
		AUDIO_OUT_SetVolume(DB.vcu.volume);

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(500));
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
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	// Initialize
	HBAR_ReadStates();

	/* Infinite loop */
	for (;;) {
		_DebugTask("Switch");

		// wait until GPIO changes
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, osWaitForever);

		if (notif & EVT_SWITCH_TRIGGERED) {
			// handle bounce effect
			osDelay(50);
			osThreadFlagsClear(EVT_MASK);

			// Read all (to handle multiple switch change at the same time)
			HBAR_ReadStates();
			// handle select & set: timer
			HBAR_CheckSelectSet();
			// Only handle Select & Set when in non-reverse
			if (!SW.list[SW_K_REVERSE].state) {
				// restore previous Mode
				HBAR_RestoreMode();
				// handle Select & Set
				if (SW.list[SW_K_SELECT].state) {
					// handle select key
					HBAR_RunSelect();
				} else if (SW.list[SW_K_SET].state) {
					// handle set key
					HBAR_RunSet();
				}
			}
		}
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
	uint32_t notif;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	for (;;) {
		_DebugTask("CanRx");

		// check if has new can message
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, osWaitForever);

		// proceed event
		if (notif & EVT_CAN_RX_IT) {
			// handle STD message
			switch (CANBUS_ReadID()) {
				//        FIXME: handle with real data
				//        case CAND_MCU_DUMMY:
				//          CANR_MCU_Dummy(&DB);
				//          break;
				case CAND_HMI2:
					CANR_HMI2(&DB);
					break;
				case CAND_HMI1_LEFT:
					CANR_HMI1_LEFT(&DB);
					break;
				case CAND_HMI1_RIGHT:
					CANR_HMI1_RIGHT(&DB);
					break;
				case CAND_BMS_PARAM_1:
					CANR_BMS_Param1(&DB);
					break;
				case CAND_BMS_PARAM_2:
					CANR_BMS_Param2(&DB);
					break;
				default:

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
	TickType_t lastWake;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	for (;;) {
		_DebugTask("CanTx");
		lastWake = osKernelGetTickCount();

		// Send CAN data
		CANT_VCU_Switch(&DB, &SW);
		CANT_VCU_RTC(&(DB.vcu.rtc.timestamp));
		CANT_VCU_SelectSet(&DB, &(SW.runner));
		CANT_VCU_TripMode(&(SW.runner.mode.sub.trip[0]));

		// Control BMS power
		if (DB.vcu.knob) {
			if (!DB_BMS_CheckRun(1) && !DB_BMS_CheckState(BMS_STATE_DISCHARGE)) {
				CANT_BMS_Setting(1, BMS_STATE_DISCHARGE);
			} else {
				// completely ON
				DB.bms.started = 1;
			}
		} else {
			if (!DB_BMS_CheckRun(0) || !DB_BMS_CheckState(BMS_STATE_IDLE)) {
				CANT_BMS_Setting(0, BMS_STATE_IDLE);
			} else {
				// completely OFF
				DB.bms.started = 0;
				DB.bms.soc = 0;
				// ohter parameter
				DB.hmi1.status.overheat = 0;
				DB.hmi1.status.warning = 1;
			}
		}

		// update BMS data
		DB_BMS_RefreshIndex();
		DB_BMS_MergeData();

		// update HMI1 data
		DB_HMI1_RefreshIndex();

		// update HMI2 data
		if ((osKernelGetTickCount() - DB.hmi2.tick) > pdMS_TO_TICKS(1000)) {
			DB.hmi2.started = 0;
		}

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(500));
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
