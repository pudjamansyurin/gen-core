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
#include "limits.h"
#include "_database.h"
#include "_config.h"
#include "_rtc.h"
#include "_crc.h"

#include "_simcom.h"
#include "_gyro.h"
#include "_gps.h"
#include "_finger.h"
#include "_audio.h"
#include "_keyless.h"
#include "_reporter.h"
#include "_can.h"
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
osMessageQId AudioVolQueueHandle;
osMutexId AudioBeepMutexHandle;
osMutexId SwvMutexHandle;
osMutexId CanTxMutexHandle;
osMutexId SimcomRecMutexHandle;
osMutexId FingerRecMutexHandle;
/* USER CODE BEGIN PV */
osMailQId GpsMailHandle;
osMailQId CommandMailHandle;
osMailQId ReportMailHandle;
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
void StartIotTask(void const *argument);
void StartGyroTask(void const *argument);
void StartCommandTask(void const *argument);
void StartGpsTask(void const *argument);
void StartFingerTask(void const *argument);
void StartAudioTask(void const *argument);
void StartKeylessTask(void const *argument);
void StartReporterTask(void const *argument);
void StartCanRxTask(void const *argument);
void StartSwitchTask(void const *argument);
void StartGeneralTask(void const *argument);
void StartCanTxTask(void const *argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern vcu_t DB_VCU;
extern hmi2_t DB_HMI2;
extern CANBUS_Rx RxCan;
extern report_t report;
extern response_t response;
extern RTC_DateTypeDef LastCalibrationDate;

const TickType_t tick5ms = pdMS_TO_TICKS(5);
const TickType_t tick100ms = pdMS_TO_TICKS(100);
const TickType_t tick250ms = pdMS_TO_TICKS(250);
const TickType_t tick500ms = pdMS_TO_TICKS(500);
const TickType_t tick1000ms = pdMS_TO_TICKS(1000);
const TickType_t tick5000ms = pdMS_TO_TICKS(5000);
const TickType_t xDelayFull_ms = pdMS_TO_TICKS(REPORT_INTERVAL_FULL*1000);
const TickType_t xDelaySimple_ms = pdMS_TO_TICKS(REPORT_INTERVAL_SIMPLE*1000);

gps_t *hGps;
command_t *hCommand;
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
	EE_Init();
	CANBUS_Init();
	/* USER CODE END 2 */
	/* Create the mutex(es) */
	/* definition and creation of AudioBeepMutex */
	osMutexDef(AudioBeepMutex);
	AudioBeepMutexHandle = osMutexCreate(osMutex(AudioBeepMutex));

	/* definition and creation of SwvMutex */
	osMutexDef(SwvMutex);
	SwvMutexHandle = osMutexCreate(osMutex(SwvMutex));

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
	/* definition and creation of AudioVolQueue */
	osMessageQDef(AudioVolQueue, 1, uint8_t);
	AudioVolQueueHandle = osMessageCreate(osMessageQ(AudioVolQueue), NULL);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	osMailQDef(GpsMail, 1, gps_t);
	GpsMailHandle = osMailCreate(osMailQ(GpsMail), NULL);
	// Allocate memory once, and never free it
	hGps = osMailAlloc(GpsMailHandle, osWaitForever);

	osMailQDef(CommandMail, 1, command_t);
	CommandMailHandle = osMailCreate(osMailQ(CommandMail), NULL);
	// Allocate memory once, and never free it
	hCommand = osMailAlloc(CommandMailHandle, osWaitForever);

	osMailQDef(ReportMail, 100, report_t);
	ReportMailHandle = osMailCreate(osMailQ(ReportMail), NULL);

	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of IotTask */
	osThreadDef(IotTask, StartIotTask, osPriorityNormal, 0, 256);
	IotTaskHandle = osThreadCreate(osThread(IotTask), NULL);

	/* definition and creation of GyroTask */
	osThreadDef(GyroTask, StartGyroTask, osPriorityNormal, 0, 256);
	GyroTaskHandle = osThreadCreate(osThread(GyroTask), NULL);

	/* definition and creation of CommandTask */
	osThreadDef(CommandTask, StartCommandTask, osPriorityAboveNormal, 0, 256);
	CommandTaskHandle = osThreadCreate(osThread(CommandTask), NULL);

	/* definition and creation of GpsTask */
	osThreadDef(GpsTask, StartGpsTask, osPriorityNormal, 0, 256);
	GpsTaskHandle = osThreadCreate(osThread(GpsTask), NULL);

	/* definition and creation of FingerTask */
	osThreadDef(FingerTask, StartFingerTask, osPriorityNormal, 0, 256);
	FingerTaskHandle = osThreadCreate(osThread(FingerTask), NULL);

	/* definition and creation of AudioTask */
	osThreadDef(AudioTask, StartAudioTask, osPriorityNormal, 0, 128);
	AudioTaskHandle = osThreadCreate(osThread(AudioTask), NULL);

	/* definition and creation of KeylessTask */
	osThreadDef(KeylessTask, StartKeylessTask, osPriorityAboveNormal, 0, 256);
	KeylessTaskHandle = osThreadCreate(osThread(KeylessTask), NULL);

	/* definition and creation of ReporterTask */
	osThreadDef(ReporterTask, StartReporterTask, osPriorityNormal, 0, 512);
	ReporterTaskHandle = osThreadCreate(osThread(ReporterTask), NULL);

	/* definition and creation of CanRxTask */
	osThreadDef(CanRxTask, StartCanRxTask, osPriorityRealtime, 0, 128);
	CanRxTaskHandle = osThreadCreate(osThread(CanRxTask), NULL);

	/* definition and creation of SwitchTask */
	osThreadDef(SwitchTask, StartSwitchTask, osPriorityNormal, 0, 128);
	SwitchTaskHandle = osThreadCreate(osThread(SwitchTask), NULL);

	/* definition and creation of GeneralTask */
	osThreadDef(GeneralTask, StartGeneralTask, osPriorityNormal, 0, 128);
	GeneralTaskHandle = osThreadCreate(osThread(GeneralTask), NULL);

	/* definition and creation of CanTxTask */
	osThreadDef(CanTxTask, StartCanTxTask, osPriorityRealtime, 0, 128);
	CanTxTaskHandle = osThreadCreate(osThread(CanTxTask), NULL);

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
	uint32_t RTC_Reset_Trigger = 888;
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
	//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0);  // uncomment this to reset RTC
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != RTC_Reset_Trigger) {
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
		// write backup register for the 1st time
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_Reset_Trigger);
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
	HAL_GPIO_WritePin(GPIOC, INT_KEYLESS_CE_Pin | INT_GPS_PWR_Pin | EXT_FINGER_PWR_Pin | EXT_HMI1_PWR_Pin
			| EXT_HMI2_PWR_Pin | INT_AUDIO_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, INT_NET_PWR_Pin | EXT_FINGER_TOUCH_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, EXT_SOLENOID_PWR_Pin | INT_NET_DTR_Pin | INT_GYRO_PWR_Pin | INT_KEYLESS_PWR_Pin
			| EXT_KEYLESS_ALARM_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_SET);

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

	/*Configure GPIO pins : INT_KEYLESS_CE_Pin INT_NET_PWR_Pin INT_GPS_PWR_Pin EXT_FINGER_TOUCH_PWR_Pin
	 EXT_FINGER_PWR_Pin EXT_HMI1_PWR_Pin EXT_HMI2_PWR_Pin INT_AUDIO_PWR_Pin */
	GPIO_InitStruct.Pin = INT_KEYLESS_CE_Pin | INT_NET_PWR_Pin | INT_GPS_PWR_Pin | EXT_FINGER_TOUCH_PWR_Pin
			| EXT_FINGER_PWR_Pin | EXT_HMI1_PWR_Pin | EXT_HMI2_PWR_Pin | INT_AUDIO_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
		if (GPIO_Pin == INT_KEYLESS_IRQ_Pin) {
			KEYLESS_IrqHandler();
		}
		// handle Finger-print IRQ
		if (GPIO_Pin == EXT_FINGER_IRQ_Pin) {
			xTaskNotifyFromISR(
					FingerTaskHandle,
					EVENT_FINGER_PLACED,
					eSetBits,
					&xHigherPriorityTaskWoken);
		}
		// handle Switches EXTI
		for (i = 0; i < DB_VCU.sw.count; i++) {
			if (GPIO_Pin == DB_VCU.sw.list[i].pin) {
				xTaskNotifyFromISR(
						SwitchTaskHandle,
						(uint32_t ) GPIO_Pin,
						eSetBits,
						&xHigherPriorityTaskWoken);

				break;
			}
		}
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
void StartIotTask(void const *argument)
{
	/* USER CODE BEGIN 5 */
	TickType_t last_wake;
	osEvent evt;
	report_t *the_report;
	ack_t the_ack;
	uint8_t success, report_size, seq, retry = 3;
	uint32_t notif_value;
	char *p_report = NULL, *p_response = NULL;

	// Start simcom module
	SIMCOM_DMA_Init();
	Simcom_Init();

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// get event data
		xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, tick100ms);

		// Set default
		success = 1;

		osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);
		// check every event & send
		if (p_response || notif_value & EVENT_IOT_RESPONSE) {
			// calculate the size based on message size
			report_size = sizeof(response.header) + sizeof(response.data.code) + strlen(response.data.message);
			// response frame
			p_response = (char*) &response;
			// Send to server
			success = Simcom_Upload(p_response, report_size);
			// validate ACK
			if (Simcom_Read_ACK(&the_ack, &(response.header))) {
				p_response = NULL;
			}
		}

		// report frame
		if (p_report || notif_value & EVENT_IOT_REPORT) {
			// check report log
			if (p_report == NULL) {
				evt = osMailGet(ReportMailHandle, 0);
			}

			if (evt.status == osEventMail) {
				seq = 0;

				do {
					// copy the pointer
					the_report = evt.value.p;
					// calculate the size based on frame_id
					report_size = sizeof(report.header) + sizeof(report.data.req);
					if (the_report->header.frame_id == FRAME_FULL) {
						report_size += sizeof(report.data.opt);
					}
					// get current sending datetime
					the_report->data.req.rtc_send_datetime = RTC_Read();
					// recalculate the CRC
					the_report->header.crc = CRC_Calculate8(
							(uint8_t*) &(the_report->header.size),
							the_report->header.size + sizeof(the_report->header.size), 1);

					// report frame
					p_report = (char*) the_report;

					// Send to server
					success = Simcom_Upload(p_report, report_size);

					// handle SIMCOM response
					if (success) {
						// validate ACK
						if (Simcom_Read_ACK(&the_ack, &(the_report->header))) {
							// Free the pointer after successfully sent
							osMailFree(ReportMailHandle, the_report);

							// Get next log (if any)
							evt = osMailGet(ReportMailHandle, 0);
							if (evt.status == osEventMail) {
								p_report = (char*) evt.value.p;
							} else {
								p_report = NULL;
							}

							// exit
							break;
						}
					}
				} while (++seq < retry);
			}
		}
		osRecursiveMutexRelease(SimcomRecMutexHandle);

		// save the simcom event
		Reporter_Set_Event(REPORT_NETWORK_RESTART, !success);

		// handle sending error
		if (!success) {
			// restart module
			BSP_LedDisco(1000);
			Simcom_Init();
		}
	}

	// Periodic interval
	vTaskDelayUntil(&last_wake, tick1000ms);
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartGyroTask */
/**
 * @brief Function implementing the gyroTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGyroTask */
void StartGyroTask(void const *argument)
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
	WaveBeepPlay(BEEP_FREQ_2000_HZ, 100);

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// Read all accelerometer, gyroscope (average)
		mems_decision = GYRO_Decision(&mems_calibration, 25);

		// Check accelerometer, happens when impact detected
		if (mems_decision.crash) {
			xTaskNotify(ReporterTaskHandle, EVENT_REPORTER_CRASH, eSetBits);
		}

		// Check gyroscope, happens when fall detected
		if (mems_decision.fall) {
			xTaskNotify(ReporterTaskHandle, EVENT_REPORTER_FALL, eSetBits);
			WaveBeepPlay(BEEP_FREQ_2000_HZ, 0);
		} else {
			xTaskNotify(ReporterTaskHandle, EVENT_REPORTER_FALL_FIXED, eSetBits);
			WaveBeepStop();
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
void StartCommandTask(void const *argument)
{
	/* USER CODE BEGIN StartCommandTask */
	command_t *command;
	osEvent evt;
	int p;

	// reset response frame to default
	Reporter_Reset(FRAME_RESPONSE);

	/* Infinite loop */
	for (;;) {
		// get command in queue
		evt = osMailGet(CommandMailHandle, tick500ms);

		if (evt.status == osEventMail) {
			command = evt.value.p;
			// default command response
			response.data.code = RESPONSE_STATUS_OK;
			strcpy(response.data.message, "");

			// handle the command
			switch (command->data.code) {
				case CMD_CODE_GEN:
					switch (command->data.sub_code) {
						case CMD_GEN_INFO:
							sprintf(response.data.message, "VCU v."VCU_FIRMWARE_VERSION", "VCU_VENDOR" @ 20%d", VCU_BUILD_YEAR);
							break;

						case CMD_GEN_LED:
							BSP_LedWrite((uint8_t) command->data.value);
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_HMI2:
					switch (command->data.sub_code) {
						case CMD_HMI2_SHUTDOWN:
							DB_HMI2.shutdown = (uint8_t) command->data.value;
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_REPORT:
					switch (command->data.sub_code) {
						case CMD_REPORT_RTC:
							RTC_Write(command->data.value);
							break;

						case CMD_REPORT_ODOM:
							Reporter_Set_Odometer((uint32_t) command->data.value);
							break;

						case CMD_REPORT_UNITID:
							Reporter_Set_UnitID((uint32_t) command->data.value);
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_AUDIO:
					switch (command->data.sub_code) {
						case CMD_AUDIO_BEEP:
							xTaskNotify(AudioTaskHandle, EVENT_AUDIO_BEEP, eSetBits);
							break;

						case CMD_AUDIO_MUTE:
							xTaskNotify(
									AudioTaskHandle,
									(uint8_t) command->data.value ? EVENT_AUDIO_MUTE_ON : EVENT_AUDIO_MUTE_OFF,
									eSetBits);
							break;

						case CMD_AUDIO_VOL:
							osMessagePut(
									AudioVolQueueHandle,
									(uint8_t) command->data.value,
									osWaitForever);
							break;

						default:
							response.data.code = RESPONSE_STATUS_INVALID;
							break;
					}
					break;

				case CMD_CODE_FINGER:
					switch (command->data.sub_code) {
						case CMD_FINGER_ADD:
							p = Finger_Enroll((uint8_t) command->data.value);
							break;

						case CMD_FINGER_DEL:
							p = Finger_Delete_ID((uint8_t) command->data.value);
							break;

						case CMD_FINGER_RST:
							p = Finger_Empty_Database();
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
			Reporter_Capture(FRAME_RESPONSE);

			// Report is ready, do what you want (send to server)
			xTaskNotify(IotTaskHandle, EVENT_IOT_RESPONSE, eSetBits);
		} else {
			// handle command (if any)
			if (Simcom_Read_Command(hCommand)) {
				osMailPut(CommandMailHandle, hCommand);
			}
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
void StartGpsTask(void const *argument)
{
	/* USER CODE BEGIN StartGpsTask */
	TickType_t last_wake;

	// Start GPS module
	UBLOX_DMA_Init();
	GPS_Init();

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// get GPS info
		if (GPS_Process(hGps)) {
			osMailPut(GpsMailHandle, hGps);
		}

		// Report interval
		vTaskDelayUntil(&last_wake, xDelayFull_ms);
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
void StartFingerTask(void const *argument)
{
	/* USER CODE BEGIN StartFingerTask */
	uint32_t notif_value;

	// Initialization
	FINGER_DMA_Init();
	Finger_Init();

	/* Infinite loop */
	for (;;) {
		// check if user put finger
		xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &notif_value, portMAX_DELAY);

		// proceed event
		if (notif_value & EVENT_FINGER_PLACED) {
			if (Finger_Auth_Fast() > 0) {
				// indicator when finger is registered
				BSP_LedWrite(1);
				osDelay(1000);
			}
		}
		BSP_LedWrite(0);
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
void StartAudioTask(void const *argument)
{
	/* USER CODE BEGIN StartAudioTask */
	TickType_t last_wake;
	uint32_t notif_value;
	osEvent evt;

	/* Initialize Wave player (Codec, DMA, I2C) */
	WaveInit();
	// Play wave loop forever, handover to DMA, so CPU is free
	WavePlay();

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// do this if events occurred
		if (xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, 0) == pdTRUE) {
			// Beeping command
			if (notif_value & EVENT_AUDIO_BEEP) {
				// Beep
				WaveBeepPlay(BEEP_FREQ_2000_HZ, 250);
				osDelay(250);
				WaveBeepPlay(BEEP_FREQ_2000_HZ, 250);
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
		evt = osMessageGet(AudioVolQueueHandle, 0);
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
void StartKeylessTask(void const *argument)
{
	/* USER CODE BEGIN StartKeylessTask */
	uint8_t msg;
	uint32_t notif_value;

	// initialization
	KEYLESS_Init();

	/* Infinite loop */
	for (;;) {
		// check if has new can message
		xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, portMAX_DELAY);

		// proceed event
		if (notif_value & EVENT_KEYLESS_RX_IT) {
			msg = KEYLESS_Read_Payload();

			SWV_SendStr("NRF received packet, msg = ");
			SWV_SendHex8(msg);
			SWV_SendChar('\n');

			// indicator
			WaveBeepPlay(BEEP_FREQ_2000_HZ, (msg + 1) * 100);
			for (int i = 0; i < ((msg + 1) * 2); i++) {
				BSP_LedToggle();
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
void StartReporterTask(void const *argument)
{
	/* USER CODE BEGIN StartReporterTask */
	TickType_t last_wake, last_wake_full = 0;
	osEvent evt;
	frame_t frame;
	report_t *log_report;
	uint8_t net_restart_state;
	uint32_t notif_value;

	// reset report frame to default
	Reporter_Reset(FRAME_FULL);

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// preserve simcom reboot from events group
		net_restart_state = Reporter_Read_Event(REPORT_NETWORK_RESTART);
		// reset all events group
		Reporter_Set_Events(0);
		// re-write the simcom reboot events
		Reporter_Set_Event(REPORT_NETWORK_RESTART, net_restart_state);

		// do this if events occurred
		if (xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, 0) == pdTRUE) {
			// check & set every event
			if (notif_value & EVENT_REPORTER_CRASH) {
				Reporter_Set_Event(REPORT_BIKE_CRASHED, 1);
			}
			if ((notif_value & EVENT_REPORTER_FALL) &&
					!(notif_value & EVENT_REPORTER_FALL_FIXED)) {
				Reporter_Set_Event(REPORT_BIKE_FALLING, 1);
			}
		}

		// get processed GPS data
		evt = osMailGet(GpsMailHandle, 0);
		// break-down RAW NMEA data from GPS module
		if (evt.status == osEventMail) {
			// set GPS data
			Reporter_Set_GPS(evt.value.p);
			// set speed value (based on GPS)
			Reporter_Set_Speed(evt.value.p);
		}

		// decide full/simple frame time
		if ((last_wake - last_wake_full) >= xDelayFull_ms) {
			// capture full frame wake time
			last_wake_full = last_wake;
			// full frame
			frame = FRAME_FULL;
		} else {
			// simple frame
			frame = FRAME_SIMPLE;
		}

		// Get current snapshot
		Reporter_Capture(frame);

		// Allocate memory, free on IoTTask after successfully sent
		log_report = osMailAlloc(ReportMailHandle, osWaitForever);

		// check log capacity
		while (log_report == NULL) {
			// get first queue
			evt = osMailGet(ReportMailHandle, 0);

			// remove the first queue
			if (evt.status == osEventMail) {
				osMailFree(ReportMailHandle, evt.value.p);
			}

			// allocate again
			log_report = osMailAlloc(ReportMailHandle, osWaitForever);
		}

		// Copy snapshot of current report
		*log_report = report;

		// Put report to log
		osMailPut(ReportMailHandle, log_report);

		// Report is ready, do what you want (send to server)
		xTaskNotify(IotTaskHandle, EVENT_IOT_REPORT, eSetBits);

		// Report interval in second (based on lowest interval, the simple frame)
		vTaskDelayUntil(&last_wake, xDelaySimple_ms);
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
void StartCanRxTask(void const *argument)
{
	/* USER CODE BEGIN StartCanRxTask */
	uint32_t notif_value;

	/* Infinite loop */
	for (;;) {
		// check if has new can message
		xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, portMAX_DELAY);

		// proceed event
		if (notif_value & EVENT_CAN_RX_IT) {
			// handle message
			switch (RxCan.RxHeader.StdId) {
				case CAN_ADDR_MCU_DUMMY:
					CAN_MCU_Dummy_Read();
					// set volume by speed
					osMessagePut(AudioVolQueueHandle, DB_VCU.speed, osWaitForever);
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
void StartSwitchTask(void const *argument)
{
	/* USER CODE BEGIN StartSwitchTask */
	uint8_t i, Last_Mode_Drive;
	uint32_t notif_value;

	// Read all EXTI state
	for (i = 0; i < DB_VCU.sw.count; i++) {
		DB_VCU.sw.list[i].state = HAL_GPIO_ReadPin(DB_VCU.sw.list[i].port, DB_VCU.sw.list[i].pin);
	}

	// Handle Reverse mode on init
	if (DB_VCU.sw.list[SW_K_REVERSE].state) {
		// save previous Drive Mode state
		if (DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] != SW_M_DRIVE_R) {
			Last_Mode_Drive = DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE];
		}
		// force state
		DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] = SW_M_DRIVE_R;
		// hazard on
		DB_VCU.sw.list[SW_K_SEIN_LEFT].state = 1;
		DB_VCU.sw.list[SW_K_SEIN_RIGHT].state = 1;
	}

	/* Infinite loop */
	for (;;) {
		xTaskNotifyStateClear(NULL);
		xTaskNotifyWait(0x00, ULONG_MAX, &notif_value, portMAX_DELAY);
		// handle bounce effect
		osDelay(50);

		// Read all (to handle multiple switch change at the same time)
		for (i = 0; i < DB_VCU.sw.count; i++) {
			DB_VCU.sw.list[i].state = HAL_GPIO_ReadPin(DB_VCU.sw.list[i].port, DB_VCU.sw.list[i].pin);

			// handle select & set: timer
			if (i == SW_K_SELECT || i == SW_K_SET) {
				// reset SET timer
				DB_VCU.sw.timer[i].time = 0;

				// next job
				if (DB_VCU.sw.list[i].state) {
					if (i == SW_K_SELECT || (i == SW_K_SET && DB_VCU.sw.runner.listening)) {
						// start timer if not running
						if (!DB_VCU.sw.timer[i].running) {
							// set flag
							DB_VCU.sw.timer[i].running = 1;
							// start timer for SET
							DB_VCU.sw.timer[i].start = osKernelSysTick();
						}
					}
					// reverse it
					DB_VCU.sw.list[i].state = 0;
				} else {
					// stop timer if running
					if (DB_VCU.sw.timer[i].running) {
						// set flag
						DB_VCU.sw.timer[i].running = 0;
						// stop SET
						DB_VCU.sw.timer[i].time = (uint8_t) ((osKernelSysTick()
								- DB_VCU.sw.timer[i].start) / tick1000ms);
						// reverse it
						DB_VCU.sw.list[i].state = 1;
					}
				}
			}
		}

		// Only handle Select & Set when in non-reverse mode
		if (DB_VCU.sw.list[SW_K_REVERSE].state) {
			// save previous Drive Mode state
			if (DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] != SW_M_DRIVE_R) {
				Last_Mode_Drive = DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE];
			}
			// force state
			DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] = SW_M_DRIVE_R;
			// hazard on
			DB_VCU.sw.list[SW_K_SEIN_LEFT].state = 1;
			DB_VCU.sw.list[SW_K_SEIN_RIGHT].state = 1;
		} else {
			// restore previous Drive Mode
			if (DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R) {
				DB_VCU.sw.runner.mode.sub.val[SW_M_DRIVE] = Last_Mode_Drive;
			}

			// handle Select & Set
			if (DB_VCU.sw.list[SW_K_SELECT].state || DB_VCU.sw.list[SW_K_SET].state) {
				// handle select key
				if (DB_VCU.sw.list[SW_K_SELECT].state) {
					if (DB_VCU.sw.runner.listening) {
						// change mode position
						if (DB_VCU.sw.runner.mode.val == SW_M_MAX) {
							DB_VCU.sw.runner.mode.val = 0;
						} else {
							DB_VCU.sw.runner.mode.val++;
						}
					}
					// Listening on option
					DB_VCU.sw.runner.listening = 1;

				} else if (DB_VCU.sw.list[SW_K_SET].state) {
					// handle set key
					if (DB_VCU.sw.runner.listening
							|| (DB_VCU.sw.timer[SW_K_SET].time >= 3 && DB_VCU.sw.runner.mode.val == SW_M_TRIP)) {
						// handle reset only if push more than n sec, and in trip mode
						if (!DB_VCU.sw.runner.listening) {
							// reset value
							DB_VCU.sw.runner.mode.sub.trip[DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val]] = 0;
						} else {
							// if less than n sec
							if (DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val]
									== DB_VCU.sw.runner.mode.sub.max[DB_VCU.sw.runner.mode.val]) {
								DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val] = 0;
							} else {
								DB_VCU.sw.runner.mode.sub.val[DB_VCU.sw.runner.mode.val]++;
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
void StartGeneralTask(void const *argument)
{
	/* USER CODE BEGIN StartGeneralTask */
	TickType_t last_wake;
	timestamp_t timestampRTC, timestampCarrier;

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// Retrieve network signal quality (every-time this task wake-up)
		Simcom_Read_Signal(&DB_VCU.signal);

		// read timestamp
		RTC_Read_RAW(&timestampRTC);
		// check calibration date (at least check every 1 day)
		if (LastCalibrationDate.Year != timestampRTC.date.Year ||
				LastCalibrationDate.Month != timestampRTC.date.Month ||
				LastCalibrationDate.Date != timestampRTC.date.Date) {

			// debugging
			SWV_SendStr("\nLastCalibrationDate : ");
			SWV_SendInt(LastCalibrationDate.Year);
			SWV_SendStr("-");
			SWV_SendInt(LastCalibrationDate.Month);
			SWV_SendStr("-");
			SWV_SendInt(LastCalibrationDate.Date);
			SWV_SendStr("\nRTCDate : ");
			SWV_SendInt(timestampRTC.date.Year);
			SWV_SendStr("-");
			SWV_SendInt(timestampRTC.date.Month);
			SWV_SendStr("-");
			SWV_SendInt(timestampRTC.date.Date);
			SWV_SendStr("");

			// get carrier timestamp
			if (Simcom_Read_Carrier_Time(&timestampCarrier)) {
				// check is carrier timestamp valid
				if (timestampCarrier.date.Year >= VCU_BUILD_YEAR) {
					// calibrate the RTC
					RTC_Write_RAW(&timestampCarrier);

					// debugging
					SWV_SendStr("\nCarrierDate : ");
					SWV_SendInt(timestampCarrier.date.Year);
					SWV_SendStr("-");
					SWV_SendInt(timestampCarrier.date.Month);
					SWV_SendStr("-");
					SWV_SendInt(timestampCarrier.date.Date);
					SWV_SendStrLn("");
				}
			}
		}

		// Periodic interval
		vTaskDelayUntil(&last_wake, tick5000ms);
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
void StartCanTxTask(void const *argument)
{
	/* USER CODE BEGIN StartCanTxTask */
	TickType_t last_wake;

	/* Infinite loop */
	last_wake = xTaskGetTickCount();
	for (;;) {
		// Send CAN data
		CAN_VCU_Switch();
		CAN_VCU_RTC();
		CAN_VCU_Select_Set();
		CAN_VCU_Trip_Mode();

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

	SWV_SendStrLn("Error Handler fired.");

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
