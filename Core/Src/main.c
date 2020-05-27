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
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "_defines.h"
#include "Parser/_at.h"
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

CRYP_HandleTypeDef hcryp;
__ALIGN_BEGIN static const uint32_t pKeyAES[4] __ALIGN_END = {
		0x00000000, 0x00000000, 0x00000000, 0x00000000 };

CAN_HandleTypeDef hcan1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

IWDG_HandleTypeDef hiwdg;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim10;

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
/* Definitions for Hmi2PowerTask */
osThreadId_t Hmi2PowerTaskHandle;
const osThreadAttr_t Hmi2PowerTask_attributes = {
		.name = "Hmi2PowerTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 128 * 4
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
/* USER CODE BEGIN PV */
osEventFlagsId_t GlobalEventHandle;
extern sw_t SW;
extern sim_t SIM;

extern vcu_t VCU;
extern bms_t BMS;
extern hmi1_t HMI1;
extern hmi2_t HMI2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_AES_Init(void);
static void MX_CAN1_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_I2S3_Init(void);
static void MX_IWDG_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RNG_Init(void);
static void MX_TIM10_Init(void);
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
	MX_ADC1_Init();
	MX_AES_Init();
	MX_CAN1_Init();
	MX_CRC_Init();
	MX_I2C1_Init();
	MX_I2C2_Init();
	MX_I2C3_Init();
	MX_I2S3_Init();
	MX_IWDG_Init();
	MX_RTC_Init();
	MX_SPI1_Init();
	MX_UART4_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_RNG_Init();
	MX_TIM10_Init();
	/* USER CODE BEGIN 2 */
	CANBUS_Init();
	BAT_DMA_Init();
	AES_Init();
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

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
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
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 100;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
			{
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S_APB1 | RCC_PERIPHCLK_RTC
			| RCC_PERIPHCLK_CLK48;
	PeriphClkInitStruct.PLLI2S.PLLI2SN = 256;
	PeriphClkInitStruct.PLLI2S.PLLI2SM = 8;
	PeriphClkInitStruct.PLLI2S.PLLI2SR = 5;
	PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
	PeriphClkInitStruct.PLLI2SSelection = RCC_PLLI2SCLKSOURCE_PLLSRC;
	PeriphClkInitStruct.I2sApb1ClockSelection = RCC_I2SAPB1CLKSOURCE_PLLI2S;
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
 * @brief AES Initialization Function
 * @param None
 * @retval None
 */
static void MX_AES_Init(void)
{

	/* USER CODE BEGIN AES_Init 0 */

	/* USER CODE END AES_Init 0 */

	/* USER CODE BEGIN AES_Init 1 */

	/* USER CODE END AES_Init 1 */
	hcryp.Instance = AES;
	hcryp.Init.DataType = CRYP_DATATYPE_8B;
	hcryp.Init.KeySize = CRYP_KEYSIZE_128B;
	hcryp.Init.pKey = (uint32_t*) pKeyAES;
	hcryp.Init.Algorithm = CRYP_AES_ECB;
	hcryp.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
	hcryp.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
	if (HAL_CRYP_Init(&hcryp) != HAL_OK)
			{
		Error_Handler();
	}
	/* USER CODE BEGIN AES_Init 2 */

	/* USER CODE END AES_Init 2 */

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
	hcan1.Init.Prescaler = 10;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
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
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
			{
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
			{
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
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
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
			{
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
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
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
			{
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
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
 * @brief RNG Initialization Function
 * @param None
 * @retval None
 */
static void MX_RNG_Init(void)
{

	/* USER CODE BEGIN RNG_Init 0 */

	/* USER CODE END RNG_Init 0 */

	/* USER CODE BEGIN RNG_Init 1 */

	/* USER CODE END RNG_Init 1 */
	hrng.Instance = RNG;
	if (HAL_RNG_Init(&hrng) != HAL_OK)
			{
		Error_Handler();
	}
	/* USER CODE BEGIN RNG_Init 2 */

	/* USER CODE END RNG_Init 2 */

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
 * @brief TIM10 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM10_Init(void)
{

	/* USER CODE BEGIN TIM10_Init 0 */

	/* USER CODE END TIM10_Init 0 */

	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM10_Init 1 */

	/* USER CODE END TIM10_Init 1 */
	htim10.Instance = TIM10;
	htim10.Init.Prescaler = 36630 - 1;
	htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim10.Init.Period = 36630 - 1;
	htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
			{
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
			{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM2;
	sConfigOC.Pulse = 36630 / 2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
			{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM10_Init 2 */

	/* USER CODE END TIM10_Init 2 */
	HAL_TIM_MspPostInit(&htim10);

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
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

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
	HAL_GPIO_WritePin(GPIOC, INT_KEYLESS_CE_Pin | INT_NET_PWR_Pin | INT_GPS_PWR_Pin | EXT_FINGER_TOUCH_PWR_Pin
			| EXT_HMI1_PWR_Pin | INT_GPS_SLEEP_Pin | EXT_HORN_PWR_Pin | INT_AUDIO_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, EXT_FINGER_PWR_Pin | EXT_HMI2_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, INT_NET_RST_Pin | INT_NET_DTR_Pin | INT_GYRO_PWR_Pin | INT_KEYLESS_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, EXT_GPIO_OUT1_Pin | SYS_LED_Pin | INT_CAN_PWR_Pin | INT_AUDIO_RST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(INT_KEYLESS_CSN_GPIO_Port, INT_KEYLESS_CSN_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, EXT_BMS_WAKEUP_Pin | EXT_BMS_FAN_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : EXT_HBAR_SELECT_Pin EXT_HBAR_SET_Pin EXT_HBAR_REVERSE_Pin EXT_ABS_IRQ_Pin
	 EXT_KNOB_IRQ_Pin EXT_STARTER_IRQ_Pin EXT_HBAR_LAMP_Pin EXT_REG_5V_IRQ_Pin
	 EXT_HBAR_SEIN_L_Pin EXT_HBAR_SEIN_R_Pin */
	GPIO_InitStruct.Pin = EXT_HBAR_SELECT_Pin | EXT_HBAR_SET_Pin | EXT_HBAR_REVERSE_Pin | EXT_ABS_IRQ_Pin
			| EXT_KNOB_IRQ_Pin | EXT_STARTER_IRQ_Pin | EXT_HBAR_LAMP_Pin | EXT_REG_5V_IRQ_Pin
			| EXT_HBAR_SEIN_L_Pin | EXT_HBAR_SEIN_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : PE4 PE10 PE12 */
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_10 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : INT_KEYLESS_CE_Pin INT_NET_PWR_Pin INT_GPS_PWR_Pin EXT_FINGER_TOUCH_PWR_Pin
	 EXT_HMI1_PWR_Pin INT_AUDIO_PWR_Pin */
	GPIO_InitStruct.Pin = INT_KEYLESS_CE_Pin | INT_NET_PWR_Pin | INT_GPS_PWR_Pin | EXT_FINGER_TOUCH_PWR_Pin
			| EXT_HMI1_PWR_Pin | INT_AUDIO_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : EXT_FINGER_PWR_Pin EXT_HMI2_PWR_Pin */
	GPIO_InitStruct.Pin = EXT_FINGER_PWR_Pin | EXT_HMI2_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB12 PB13 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_12 | GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : EXT_SOLENOID_PWR_Pin */
	GPIO_InitStruct.Pin = EXT_SOLENOID_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EXT_SOLENOID_PWR_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : EXT_FINGER_IRQ_Pin */
	GPIO_InitStruct.Pin = EXT_FINGER_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(EXT_FINGER_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : INT_GYRO_IRQ_Pin */
	GPIO_InitStruct.Pin = INT_GYRO_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(INT_GYRO_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : INT_KEYLESS_IRQ_Pin */
	GPIO_InitStruct.Pin = INT_KEYLESS_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(INT_KEYLESS_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : INT_NET_RST_Pin INT_NET_DTR_Pin INT_GYRO_PWR_Pin INT_KEYLESS_PWR_Pin */
	GPIO_InitStruct.Pin = INT_NET_RST_Pin | INT_NET_DTR_Pin | INT_GYRO_PWR_Pin | INT_KEYLESS_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : EXT_GPIO_OUT1_Pin SYS_LED_Pin INT_AUDIO_RST_Pin */
	GPIO_InitStruct.Pin = EXT_GPIO_OUT1_Pin | SYS_LED_Pin | INT_AUDIO_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : PD9 PD10 PD11 PD12
	 PD14 PD15 PD2 PD7 */
	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
			| GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_2 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : INT_GPS_SLEEP_Pin EXT_HORN_PWR_Pin */
	GPIO_InitStruct.Pin = INT_GPS_SLEEP_Pin | EXT_HORN_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA11 PA12 */
	GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : INT_KEYLESS_CSN_Pin */
	GPIO_InitStruct.Pin = INT_KEYLESS_CSN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(INT_KEYLESS_CSN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : INT_CAN_PWR_Pin */
	GPIO_InitStruct.Pin = INT_CAN_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(INT_CAN_PWR_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : EXT_BMS_WAKEUP_Pin EXT_BMS_FAN_PWR_Pin */
	GPIO_InitStruct.Pin = EXT_BMS_WAKEUP_Pin | EXT_BMS_FAN_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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
		if (GPIO_Pin == EXT_REG_5V_IRQ_Pin) {
			osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_REG_5V_IRQ);
		}
		// handle KNOB IRQ (Power control for HMI1 & HMI2)
		if (GPIO_Pin == EXT_KNOB_IRQ_Pin) {
			osThreadFlagsSet(SwitchTaskHandle, EVT_SWITCH_KNOB_IRQ);
		}
		// handle Finger IRQ
		if (GPIO_Pin == EXT_FINGER_IRQ_Pin) {
			osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_PLACED);
		}
		// handle NRF24 IRQ
		if (GPIO_Pin == INT_KEYLESS_IRQ_Pin) {
			KLESS_IrqHandler();
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
	TickType_t lastWake;

	// Initialization, this task get executed first!
	VCU.Init();
	BMS.Init();
	HMI1.Init();
	HMI2.Init();
	EEPROM_ResetOrLoad();

	// Check GPIOs state
	VCU.CheckMainPower();
	VCU.d.state.knob = HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);

	// Threads management:
	//	osThreadSuspend(IotTaskHandle);
	//	osThreadSuspend(ReporterTaskHandle);
	//	osThreadSuspend(CommandTaskHandle);
	//	osThreadSuspend(GpsTaskHandle);
	//	osThreadSuspend(GyroTaskHandle);
	//	osThreadSuspend(KeylessTaskHandle);
	//	osThreadSuspend(FingerTaskHandle);
	//	osThreadSuspend(AudioTaskHandle);
	//	osThreadSuspend(SwitchTaskHandle);
	//	osThreadSuspend(CanRxTaskHandle);
	//	osThreadSuspend(CanTxTaskHandle);
	//	osThreadSuspend(Hmi2PowerTaskHandle);

	// Release threads
	osEventFlagsSet(GlobalEventHandle, EVENT_READY);

	/* Infinite loop */
	for (;;) {
		lastWake = osKernelGetTickCount();

		// Feed the dog
		HAL_IWDG_Refresh(&hiwdg);

		// Dummy data generator
		_DummyGenerator();

		// Battery Monitor
		BAT_Debugger();

		// Thread's Stack Monitor
		_RTOS_Debugger(10000);

		// Check is Daylight
		HMI1.d.status.daylight = RTC_IsDaylight(VCU.d.rtc.timestamp);

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(1000));
	}
	/* USER CODE END 5 */
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
	osStatus_t status;
	report_t report;
	response_t response;
	uint8_t retry, nack, pending[2] = { 0 };
	uint32_t notif;

	SIMCOM_RESULT p;
	at_csq_t signal;

	osMessageQueueId_t *pQueue;
	header_t *pHeader;
	void *pPayload;
	const uint8_t size = sizeof(report.header.prefix) +
			sizeof(report.header.crc) +
			sizeof(report.header.size);

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Start simcom module
	SIMCOM_DMA_Init();

	/* Infinite loop */
	for (;;) {
		lastWake = osKernelGetTickCount();

		// Upload Report & Response Payload
		if (Simcom_SetState(SIM_STATE_SERVER_ON)) {
			// Iterate between REPORT & RESPONSE
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
						// Re-calculate CRC
						if (type == PAYLOAD_REPORT) {
							Report_SetCRC((report_t*) pPayload);
						} else {
							Response_SetCRC((response_t*) pPayload);
						}

						// Send to server
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

				// Handle Full Buffer
				if (type == PAYLOAD_REPORT) {
					notif = osThreadFlagsWait(EVT_IOT_DISCARD, osFlagsWaitAny, 0);
					if (_RTOS_ValidThreadFlag(notif)) {
						pending[type] = 0;
					}
				}
			}
		}

		// ================= SIMCOM Related Routines ================
		if (Simcom_SetState(SIM_STATE_READY)) {
			if (AT_SignalQualityReport(&signal)) {
				VCU.d.signal = signal.percent;
			}
			if (RTC_NeedCalibration()) {
				RTC_Calibrate();
			}
		}

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(500));
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
	TickType_t lastWake;
	report_t report;
	osStatus_t status;
	FRAME_TYPE frame;
	uint8_t frameDecider = 0;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialize
	Report_Init(FR_SIMPLE, &report);

	/* Infinite loop */
	for (;;) {
		lastWake = osKernelGetTickCount();

		// Frame type decider
		if (!VCU.d.state.independent) {
			if (++frameDecider == RPT_INTERVAL_FULL_AT_SIMPLE) {
				frame = FR_FULL;
				frameDecider = 0;
			} else {
				frame = FR_SIMPLE;
			}
		} else {
			frame = FR_FULL;
			frameDecider = 0;
		}

		// Get current snapshot
		Report_Capture(frame, &report);

		do {
			// Put report to log
			status = osMessageQueuePut(ReportQueueHandle, &report, 0U, 0U);
			// already full, remove oldest
			if (status == osErrorResource) {
				osThreadFlagsSet(IotTaskHandle, EVT_IOT_DISCARD);
			}
		} while (status != osOK);

		// reset some events group
		VCU.SetEvent(EV_VCU_NETWORK_RESTART, 0);

		// Report interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(VCU.d.interval * 1000));
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
	response_t response;
	command_t command;
	osStatus_t status;
	uint32_t notif;
	uint8_t driver;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialize
	Response_Init(&response);

	/* Infinite loop */
	for (;;) {
		// get command in queue
		status = osMessageQueueGet(CommandQueueHandle, &command, NULL, osWaitForever);

		if (status == osOK) {
			Command_Debugger(&command);

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
							VCU.d.state.knob = (uint8_t) command.data.value;
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_REPORT:
					switch (command.data.sub_code) {
						case CMD_REPORT_RTC:
							RTC_Write((uint64_t) command.data.value, &(VCU.d.rtc));
							break;

						case CMD_REPORT_ODOM:
							EEPROM_Odometer(EE_CMD_W, (uint32_t) command.data.value);
							break;

						case CMD_REPORT_UNITID:
							EEPROM_UnitID(EE_CMD_W, (uint32_t) command.data.value);
							break;

						case CMD_REPORT_INTERVAL:
							VCU.d.interval = (uint16_t) command.data.value;
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
							if ((uint8_t) command.data.value) {
								osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_MUTE_ON);
							} else {
								osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_MUTE_OFF);
							}

							break;

						case CMD_AUDIO_VOL:
							VCU.d.volume = (uint8_t) command.data.value;
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_FINGER:
					// put finger index to queue
					driver = command.data.value;
					osMessageQueuePut(DriverQueueHandle, &driver, 0U, 0U);

					switch (command.data.sub_code) {
						case CMD_FINGER_ADD:
							// do not handle if drivers is full
							if (driver < FINGER_USER_MAX) {
								osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_ADD);
							} else {
								response.data.code = RESPONSE_STATUS_ERROR;
							}
							break;

						case CMD_FINGER_DEL:
							osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_DEL);
							break;

						case CMD_FINGER_RST:
							osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_RST);
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}

					// wait response until timeout (30 seconds)
					if (response.data.code == RESPONSE_STATUS_OK) {
						notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(30000));
						if (_RTOS_ValidThreadFlag(notif)) {
							if (notif & EVT_COMMAND_ERROR) {
								response.data.code = RESPONSE_STATUS_ERROR;
							}
						}
					}
					break;

				case CMD_CODE_KEYLESS:
					switch (command.data.sub_code) {
						case CMD_KEYLESS_PAIRING:
							osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_PAIRING);

							// wait response until timeout (30 seconds)
							notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(30000));
							if (_RTOS_ValidThreadFlag(notif)) {
								if (notif & EVT_COMMAND_ERROR) {
									response.data.code = RESPONSE_STATUS_ERROR;
								}
							}

							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
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
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialize
	UBLOX_DMA_Init();
	GPS_Init();

	/* Infinite loop */
	for (;;) {
		lastWake = osKernelGetTickCount();

		GPS_Capture();
		GPS_CalculateOdometer();
		//		GPS_Debugger();

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(GPS_INTERVAL_MS));
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
	TickType_t lastWake;
	mems_decision_t decider, tmp;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* MPU6050 Initialization*/
	GYRO_Init();

	/* Infinite loop */
	for (;;) {
		lastWake = osKernelGetTickCount();

		// Read all accelerometer, gyroscope (average)
		decider = GYRO_Decision(25);
		//		Gyro_Debugger(&decider);

		// Check accelerometer, happens when impact detected
		if (tmp.crash.state != decider.crash.state) {
			tmp.crash.state = decider.crash.state;

			VCU.SetEvent(EV_VCU_BIKE_CRASHED, decider.crash.state);
		}

		// Check gyroscope, happens when fall detected
		if (tmp.fall.state != decider.fall.state) {
			tmp.fall.state = decider.fall.state;

			if (decider.fall.state) {
				osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_START);
			} else {
				osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_STOP);
			}
			VCU.SetEvent(EV_VCU_BIKE_FALLING, decider.fall.state);
			_LedWrite(decider.fall.state);
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
	uint32_t notif;
	KLESS_CMD command;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// initialization
	KLESS_Init();

	/* Infinite loop */
	for (;;) {
		// check if has new can message
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(100));
		// proceed event
		if (_RTOS_ValidThreadFlag(notif)) {
			// handle incomming payload
			if (notif & EVT_KEYLESS_RX_IT) {
				KLESS_Debugger();

				// process
				if (KLESS_ValidateCommand(&command)) {
					// handle command
					switch (command) {
						case KLESS_CMD_PING:
							// update heart-beat
							VCU.d.tick.keyless = osKernelGetTickCount();

							break;
						case KLESS_CMD_ALARM:
							// toggle the hazard
							SW.runner.hazard = 1;
							for (uint8_t i = 0; i < 2; i++) {
								// toggle HORN (+ Sein Lamp)
								HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, 1);
								osDelay(500);
								HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, 0);
								osDelay(500);
							}
							SW.runner.hazard = 0;

							break;
						case KLESS_CMD_SEAT:
							HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, 1);
							osDelay(500);
							HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, 0);
							osDelay(500);

							break;
						default:
							break;
					}

					// indicator
					osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_START);
					_LedWrite(1);
					osDelay(command * 250);
					_LedWrite(0);
					osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP_STOP);
					LOG_StrLn("NRF:Command = OK");
				} else {
					LOG_StrLn("NRF:Command = ERROR");
				}
			}

			// handle pairing
			if (notif & EVT_KEYLESS_PAIRING) {

				// handle response
				//				osThreadFlagsSet(CommandTaskHandle, p ? EVT_COMMAND_OK : EVT_COMMAND_ERROR);
			}

			// reset ThreadFlag
			osThreadFlagsClear(EVT_MASK);
		}

		// update state
		KLESS_Refresh();

		// for testing
		//		if (KLESS_SendDummy()) {
		//			LOG_StrLn("NRF:Send = OK");
		//		} else {
		//			LOG_StrLn("NRF:Send = ERROR");
		//		}
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
	osStatus_t status;
	uint32_t notif;
	uint8_t driver, p;
	int8_t id;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	// Initialization
	FINGER_DMA_Init();
	Finger_Init();

	/* Infinite loop */
	for (;;) {
		// check if user put finger
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(100));
		// proceed event
		if (_RTOS_ValidThreadFlag(notif)) {
			// Scan existing finger
			if (notif & EVT_FINGER_PLACED) {
				id = Finger_AuthFast();
				if (id >= 0) {
					// Finger is registered
					VCU.d.state.knob = !VCU.d.state.knob;

					// Finger Heart-Beat
					if (!VCU.d.state.knob) {
						id = DRIVER_ID_NONE;
					}
					VCU.d.driver_id = id;

					// Handle bounce effect
					osDelay(5000);
					osThreadFlagsClear(EVT_FINGER_PLACED);
				}
			}

			if (notif & (EVT_FINGER_ADD | EVT_FINGER_DEL | EVT_FINGER_RST)) {
				// get driver value
				status = osMessageQueueGet(DriverQueueHandle, &driver, NULL, 0U);

				if (status == osOK) {
					// Add new finger
					if (notif & EVT_FINGER_ADD) {
						p = Finger_Enroll(driver);
					}
					// Delete existing finger
					if (notif & EVT_FINGER_DEL) {
						p = Finger_DeleteID(driver);
					}
					// Reset all finger database
					if (notif & EVT_FINGER_RST) {
						p = Finger_EmptyDatabase();
					}

					// handle response
					osThreadFlagsSet(CommandTaskHandle, p ? EVT_COMMAND_OK : EVT_COMMAND_ERROR);
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
	uint32_t notif;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Initialize Wave player (Codec, DMA, I2C) */
	AUDIO_Init();
	// Play wave loop forever, hand-over to DMA, so CPU is free
	AUDIO_Play();

	/* Infinite loop */
	for (;;) {
		// wait with timeout
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny, pdMS_TO_TICKS(100));
		if (_RTOS_ValidThreadFlag(notif)) {
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
		AUDIO_OUT_SetVolume(VCU.d.volume);
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

	// Initialize
	HBAR_ReadStates();

	/* Infinite loop */
	for (;;) {
		// wait forever
		notif = osThreadFlagsWait(EVT_MASK, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
		if (_RTOS_ValidThreadFlag(notif)) {
			// handle bounce effect
			osDelay(50);
			osThreadFlagsClear(EVT_MASK);

			// Handle switch EXTI interrupt
			if (notif & EVT_SWITCH_TRIGGERED) {
				// Read all (to handle multiple switch change at the same time)
				HBAR_ReadStates();

				// handle select & set: timer
				HBAR_TimerSelectSet();

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

			// Handle other EXTI interrupt
			// BMS Power IRQ
			if (notif & EVT_SWITCH_REG_5V_IRQ) {
				// get current state
				VCU.CheckMainPower();
			}
			// KNOB IRQ
			if (notif & EVT_SWITCH_KNOB_IRQ) {
				// get current state
				VCU.d.state.knob = HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);
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
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	for (;;) {
		// wait forever
		notif = osThreadFlagsWait(EVT_CAN_RX_IT, osFlagsWaitAny, osWaitForever);
		if (_RTOS_ValidThreadFlag(notif)) {
			//			CANBUS_RxDebugger();

			// handle STD message
			switch (CANBUS_ReadID()) {
				case CAND_HMI2:
					HMI2.can.r.State();
					break;
				case CAND_HMI1_LEFT:
					HMI1.can.r.LeftState();
					break;
				case CAND_HMI1_RIGHT:
					HMI1.can.r.RightState();
					break;
				case CAND_BMS_PARAM_1:
					BMS.can.r.Param1();
					break;
				case CAND_BMS_PARAM_2:
					BMS.can.r.Param2();
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
	TickType_t lastWake, last500ms, last1000ms;

	// wait until ManagerTask done
	osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);

	/* Infinite loop */
	last500ms = osKernelGetTickCount();
	last1000ms = osKernelGetTickCount();
	for (;;) {
		lastWake = osKernelGetTickCount();

		// Send CAN data
		// send every 20m
		VCU.can.t.SwitchModeControl(&SW);

		// send every 500ms
		if (lastWake - last500ms > pdMS_TO_TICKS(500)) {
			last500ms = osKernelGetTickCount();

			VCU.can.t.MixedData(&(SW.runner));
		}
		// send every 1000ms
		if (lastWake - last1000ms > pdMS_TO_TICKS(1000)) {
			last1000ms = osKernelGetTickCount();

			VCU.can.t.Datetime(&(VCU.d.rtc.timestamp));
			VCU.can.t.SubTripData(&(SW.runner.mode.sub.trip[0]));
		}

		// Handle Knob Changes
		BMS.PowerOverCan(VCU.d.state.knob);
		HMI1.Power(VCU.d.state.knob);
		HMI2.PowerOverCan(VCU.d.state.knob);

		// Refresh state
		BMS.RefreshIndex();
		HMI1.RefreshIndex();
		HMI2.Refresh();

		// Periodic interval
		osDelayUntil(lastWake + pdMS_TO_TICKS(20));
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
__weak void StartHmi2PowerTask(void *argument)
{
	/* USER CODE BEGIN StartHmi2PowerTask */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END StartHmi2PowerTask */
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
