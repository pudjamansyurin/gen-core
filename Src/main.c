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
#include "_config.h"
#include "_reporter.h"
#include "_nmea.h"
#include "_mems.h"
#include "_simcom.h"
#include "_finger.h"
#include "_ee_emulation.h"
#include "_nrf24l01.h"
#include "_canbus.h"
#include "_database.h"
#include "_rtc.h"
#include "_audio.h"
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
osMessageQId AudioVolQueueHandle;
osTimerId TimerCANHandle;
osMutexId AudioBeepMutexHandle;
osMutexId SwvMutexHandle;
osMutexId CanTxMutexHandle;
osMutexId SimcomRecMutexHandle;
osMutexId FingerRecMutexHandle;
/* USER CODE BEGIN PV */
osMailQId GpsMailHandle;
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
void CallbackTimerCAN(void const *argument);

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
	/* USER CODE BEGIN 2 */
	EE_Init();
	CAN_Init();
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

	/* Create the timer(s) */
	/* definition and creation of TimerCAN */
	osTimerDef(TimerCAN, CallbackTimerCAN);
	TimerCANHandle = osTimerCreate(osTimer(TimerCAN), osTimerPeriodic, NULL);

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	osTimerStart(TimerCANHandle, TIMER_CAN_MS);
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* definition and creation of AudioVolQueue */
	osMessageQDef(AudioVolQueue, 1, uint8_t);
	AudioVolQueueHandle = osMessageCreate(osMessageQ(AudioVolQueue), NULL);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	osMailQDef(GpsMail, 1, gps_t);
	GpsMailHandle = osMailCreate(osMailQ(GpsMail), NULL);

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

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	// FIXME use IWDG or WWDG for auto reset on malfunction
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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
	//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0);  // uncomment this to reset RTC
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2) {
		/* USER CODE END Check_RTC_BKUP */

		/** Initialize RTC and set the Time and Date
		 */
		sTime.Hours = 0x10;
		sTime.Minutes = 0x20;
		sTime.Seconds = 0x0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;
		if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
				{
			Error_Handler();
		}
		sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
		sDate.Month = RTC_MONTH_OCTOBER;
		sDate.Date = 0x2;
		sDate.Year = 0x19;

		if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
				{
			Error_Handler();
		}
		/* USER CODE BEGIN RTC_Init 2 */
		// write backup register for the 1st time
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
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

	/*Configure GPIO pins : EXT_HBAR_SELECT_Pin EXT_HBAR_SET_Pin EXT_HMI2_PHONE_Pin EXT_HBAR_REVERSE_Pin
	 EXT_ABS_STATUS_Pin EXT_HBAR_SEIN_L_Pin EXT_HBAR_SEIN_R_Pin */
	GPIO_InitStruct.Pin = EXT_HBAR_SELECT_Pin | EXT_HBAR_SET_Pin | EXT_HMI2_PHONE_Pin | EXT_HBAR_REVERSE_Pin
			| EXT_ABS_STATUS_Pin | EXT_HBAR_SEIN_L_Pin | EXT_HBAR_SEIN_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
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

	HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void vApplicationIdleHook(void) {
	// Feed the dog
	//	HAL_IWDG_Refresh(&hiwdg);

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	extern nrf24l01 nrf;

	if (osKernelRunning()) {
		// handle NRF24 IRQ
		if (GPIO_Pin == INT_KEYLESS_IRQ_Pin) {
			nrf_irq_handler(&nrf);
		}
		// handle Fingerprint IRQ
		if (GPIO_Pin == EXT_FINGER_IRQ_Pin) {
			xTaskNotifyFromISR(
					FingerTaskHandle,
					EVENT_FINGER_PLACED,
					eSetBits,
					&xHigherPriorityTaskWoken);
		}
		// handle Handlebars
		if (GPIO_Pin == EXT_HBAR_SELECT_Pin
				|| GPIO_Pin == EXT_HBAR_SET_Pin
				|| GPIO_Pin == EXT_HMI2_PHONE_Pin
				|| GPIO_Pin == EXT_HBAR_REVERSE_Pin
				|| GPIO_Pin == EXT_ABS_STATUS_Pin
				|| GPIO_Pin == EXT_HBAR_SEIN_L_Pin
				|| GPIO_Pin == EXT_HBAR_SEIN_R_Pin) {
			xTaskNotifyFromISR(
					SwitchTaskHandle,
					(uint32_t ) GPIO_Pin,
					eSetBits,
					&xHigherPriorityTaskWoken);
		}
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xTaskNotifyFromISR(
			KeylessTaskHandle,
			EVENT_KEYLESS_RX_IT,
			eSetBits,
			&xHigherPriorityTaskWoken);

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
	extern response_t response;
	extern report_t report;
	extern uint8_t DB_VCU_Signal;
	uint32_t notifValue;
	uint8_t success, reportSize;
	osEvent evt;
	report_t *theReport;
	char *payload;

	// Start simcom module
	SIMCOM_DMA_Init();
	Simcom_Init();
	/* Infinite loop */
	for (;;) {
		// get event data
		xTaskNotifyWait(0x00, ULONG_MAX, &notifValue, portMAX_DELAY);

		// Retrieve network signal quality (every-time this task wake-up)
		Simcom_Read_Signal(&DB_VCU_Signal);

		// Set default
		success = 1;

		// check every event & send
		if (notifValue & EVENT_IOT_RESPONSE) {
			// calculate the size based on message size
			reportSize = sizeof(response.header) + sizeof(response.data.code) + strlen(response.data.message);
			// response frame
			payload = (char*) &response;
			// Send to server
			success = Simcom_Upload(payload, reportSize);

		} else {
			// report frame
			// check report log
			evt = osMailGet(ReportMailHandle, 0);

			// loop trough the log
			while (evt.status == osEventMail) {
				// copy the pointer
				theReport = evt.value.p;
				// calculate the size based on frame_id
				reportSize = sizeof(report.header) + sizeof(report.data.req);
				if (theReport->header.frame_id == FRAME_FULL) {
					reportSize += sizeof(report.data.opt);
				}
				// report frame
				payload = (char*) evt.value.p;

				// Send to server
				success = Simcom_Upload(payload, reportSize);
				// handle SIMCOM response
				if (success) {
					// Free the pointer after successfully sent
					osMailFree(ReportMailHandle, theReport);
				} else {
					// SIMCOM failed
					break;
				}

				// check report log again
				evt = osMailGet(ReportMailHandle, 0);
			};
		}

		// save the simcom event
		Reporter_Set_Event(REPORT_SIMCOM_RESTART, !success);
		// handle sending error
		if (!success) {
			// restart module
			BSP_Led_Disco(1000);
			Simcom_Init();
		}
	}
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
	const TickType_t tick5ms = pdMS_TO_TICKS(5);
	TickType_t xLastWakeTime;
	mems_t mems_calibration;
	mems_decision_t mems_decision;
	SD_MPU6050 mpu;

	/* MPU6050 Initialization*/
	MEMS_Init(&hi2c3, &mpu);
	// Set calibrator
	mems_calibration = MEMS_Average(&hi2c3, &mpu, NULL, 500);
	// Give success indicator
	WaveBeepPlay(BEEP_FREQ_2000_HZ, 100);

	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// Read all accelerometer, gyroscope (average)
		mems_decision = MEMS_Decision(&hi2c3, &mpu, &mems_calibration, 25);

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
		vTaskDelayUntil(&xLastWakeTime, tick5ms);
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
	extern response_t response;

	const TickType_t tick100ms = pdMS_TO_TICKS(100);
	TickType_t xLastWakeTime;
	BaseType_t xResult;
	command_t command;
	uint32_t ulNotifiedValue;
	uint8_t newCommand;
	int p;

	// reset response frame to default
	Reporter_Reset(FRAME_RESPONSE);

	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// reset command indicator state
		newCommand = 0;

		// check if command arrived from IOT Task
		xResult = xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, 0);
		if (xResult == pdTRUE) {
			if ((ulNotifiedValue & EVENT_COMMAND_ARRIVED)) {
				newCommand = 1;
			}
		}

		// if there is no pending command, fetch the newest command
		if (!newCommand) {
			newCommand = Simcom_Check_Command();
		}

		// then execute the command
		if (newCommand) {
			SWV_SendStr("\nNew Command: from CommandTask");
			// read the command & execute
			if (Simcom_Read_Command(&command)) {
				// default command response
				response.data.code = RESPONSE_STATUS_OK;
				strcpy(response.data.message, "");

				// handle the command
				switch (command.data.code) {
					case CMD_CODE_GEN:
						switch (command.data.sub_code) {
							case CMD_GEN_INFO:
								strcpy(
										response.data.message,
										"VCU v."VCU_FIRMWARE_VERSION", "VCU_VENDOR" @ "VCU_BUILD_YEAR);
								break;

							case CMD_GEN_LED:
								BSP_Led_Write((uint8_t) command.data.value);
								break;

							default:
								response.data.code = RESPONSE_STATUS_INVALID;
								break;
						}
						break;

					case CMD_CODE_REPORT:
						switch (command.data.sub_code) {
							case CMD_REPORT_RTC:
								// FIXME:
								SWV_SendStr("\nRTC = ");
								SWV_SendBufHex((char*) &(command.data.value), sizeof(command.data.value));
								SWV_SendStr("\n");
								//RTC_Write(command.data.value);
								break;

							case CMD_REPORT_ODOM:
								Reporter_Set_Odometer((uint32_t) command.data.value);
								break;

							case CMD_REPORT_UNITID:
								Reporter_Set_UnitID((uint32_t) command.data.value);
								break;

							default:
								response.data.code = RESPONSE_STATUS_INVALID;
								break;
						}
						break;

					case CMD_CODE_AUDIO:
						switch (command.data.sub_code) {
							case CMD_AUDIO_BEEP:
								xTaskNotify(AudioTaskHandle, EVENT_AUDIO_BEEP, eSetBits);
								break;

							case CMD_AUDIO_MUTE:
								xTaskNotify(
										AudioTaskHandle,
										(uint8_t) command.data.value ? EVENT_AUDIO_MUTE_ON : EVENT_AUDIO_MUTE_OFF,
										eSetBits);
								break;

							case CMD_AUDIO_VOL:
								osMessagePut(
										AudioVolQueueHandle,
										(uint8_t) command.data.value,
										osWaitForever);
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
								p = Finger_Delete_ID((uint8_t) command.data.value);
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

				// Set header
				Reporter_Set_Header(FRAME_RESPONSE);

				// Report is ready, do what you want (send to server)
				xTaskNotify(IotTaskHandle, EVENT_IOT_RESPONSE, eSetBits);
			}
		}

		// Report interval
		vTaskDelayUntil(&xLastWakeTime, tick100ms);
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
	extern char UBLOX_UART_RX_Buffer[UBLOX_UART_RX_BUFFER_SIZE];
	const TickType_t xDelay_ms = pdMS_TO_TICKS(REPORT_INTERVAL_FULL * 1000);
	TickType_t xLastWakeTime;
	gps_t *hgps;

	// Start GPS module
	UBLOX_DMA_Init();
	// Allocate memory once, and never free it
	hgps = osMailAlloc(GpsMailHandle, osWaitForever);
	Ublox_Init(hgps);

	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// get GPS info
		gps_process(hgps, UBLOX_UART_RX_Buffer, strlen(UBLOX_UART_RX_Buffer));

		// hand-over data to IOT_Task (if fixed)
		if (hgps->fix > 0) {
			osMailPut(GpsMailHandle, hgps);
		}

		// Report interval
		vTaskDelayUntil(&xLastWakeTime, xDelay_ms);
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
	uint32_t ulNotifiedValue;

	// Initialization
	FINGER_DMA_Init();
	Finger_Init();

	/* Infinite loop */
	for (;;) {
		// check if user put finger
		xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

		// proceed event
		if ((ulNotifiedValue & EVENT_FINGER_PLACED)) {
			if (Finger_Auth_Fast() > 0) {
				// indicator when finger is registered
				BSP_Led_Write(1);
				osDelay(1000);
			}
		}
		BSP_Led_Write(0);
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
	const TickType_t tick500ms = pdMS_TO_TICKS(500);
	TickType_t xLastWakeTime;
	uint32_t ulNotifiedValue;
	BaseType_t xResult;
	osEvent evt;
	/* Initialize Wave player (Codec, DMA, I2C) */
	WaveInit();
	// Play wave loop forever, handover to DMA, so CPU is free
	WavePlay();

	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// check if event happen
		xResult = xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, 0);
		// do this if events occurred
		if (xResult == pdTRUE) {
			// Beep command
			if ((ulNotifiedValue & EVENT_AUDIO_BEEP)) {
				// Beep
				WaveBeepPlay(BEEP_FREQ_2000_HZ, 250);
				osDelay(250);
				WaveBeepPlay(BEEP_FREQ_2000_HZ, 250);
			}
			// Mute command
			if ((ulNotifiedValue & EVENT_AUDIO_MUTE_ON)) {
				AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
			}
			if ((ulNotifiedValue & EVENT_AUDIO_MUTE_OFF)) {
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
		vTaskDelayUntil(&xLastWakeTime, tick500ms);
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
	extern nrf24l01 nrf;
	nrf24l01_config config;
	uint8_t msg;
	uint8_t payload_length = 8;
	uint8_t payload_rx[payload_length];
	uint32_t ulNotifiedValue;

	// set configuration
	nrf_set_config(&config, payload_rx, payload_length);
	// initialization
	nrf_init(&nrf, &config);
	SWV_SendStrLn("NRF24_Init");

	/* Infinite loop */
	for (;;) {
		// check if has new can message
		xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

		// proceed event
		if ((ulNotifiedValue & EVENT_KEYLESS_RX_IT)) {
			msg = payload_rx[payload_length - 1];

			SWV_SendStr("NRF received packet, msg = ");
			SWV_SendHex8(msg);
			SWV_SendChar('\n');

			// indicator
			WaveBeepPlay(BEEP_FREQ_2000_HZ, (msg + 1) * 100);
			for (int i = 0; i < ((msg + 1) * 2); i++) {
				BSP_Led_Toggle();
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
	const TickType_t xDelaySimple_ms = pdMS_TO_TICKS(REPORT_INTERVAL_SIMPLE*1000);
	const TickType_t xDelayFull_ms = pdMS_TO_TICKS(REPORT_INTERVAL_FULL*1000);
	TickType_t xLastWakeTime, xLastFullWakeTime = 0;
	BaseType_t xResult;
	uint8_t simcomRestartState;
	uint32_t ulNotifiedValue;
	osEvent evt;
	frame_t frame;
	report_t *logReport;
	extern report_t report;

	// reset report frame to default
	Reporter_Reset(FRAME_FULL);

	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// preserve simcom reboot from events group
		simcomRestartState = Reporter_Read_Event(REPORT_SIMCOM_RESTART);
		// reset all events group
		report.data.req.events_group = 0;
		// re-write the simcom reboot events
		Reporter_Set_Event(REPORT_SIMCOM_RESTART, simcomRestartState);

		// get event data
		xResult = xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, 0);
		// do this if events occurred
		if (xResult == pdTRUE) {
			// check & set every event
			if (ulNotifiedValue & EVENT_REPORTER_CRASH) {
				Reporter_Set_Event(REPORT_BIKE_CRASHED, 1);
			}
			if ((ulNotifiedValue & EVENT_REPORTER_FALL) &&
					!(ulNotifiedValue & EVENT_REPORTER_FALL_FIXED)) {
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

		if ((xLastWakeTime - xLastFullWakeTime) >= xDelayFull_ms) {
			// capture full frame wake time
			xLastFullWakeTime = xLastWakeTime;
			// full frame
			frame = FRAME_FULL;
		} else {
			// simple frame
			frame = FRAME_SIMPLE;
		}

		// Set header
		Reporter_Set_Header(frame);

		// Allocate memory, free on IoTTask after successfully sent
		logReport = osMailAlloc(ReportMailHandle, osWaitForever);

		// check log capacity
		while (logReport == NULL) {
			// get first queue
			evt = osMailGet(ReportMailHandle, 0);

			// remove the first queue
			if (evt.status == osEventMail) {
				osMailFree(ReportMailHandle, evt.value.p);
			}

			// allocate again
			logReport = osMailAlloc(ReportMailHandle, osWaitForever);
		}

		// Copy snapshot of current report
		*logReport = report;

		// Put report to log
		osMailPut(ReportMailHandle, logReport);

		// Report is ready, do what you want (send to server)
		xTaskNotify(IotTaskHandle, EVENT_IOT_REPORT, eSetBits);

		// Report interval in second (based on lowest interval, the simple frame)
		vTaskDelayUntil(&xLastWakeTime, xDelaySimple_ms);
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
	extern uint8_t DB_VCU_Speed;
	extern CAN_Rx RxCan;
	uint32_t ulNotifiedValue;

	/* Infinite loop */
	for (;;) {
		// check if has new can message
		xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

		// proceed event
		if ((ulNotifiedValue & EVENT_CAN_RX_IT)) {
			// handle message
			switch (RxCan.RxHeader.StdId) {
				case CAN_ADDR_MCU_DUMMY:
					CANBUS_MCU_Dummy_Read();
					// set volume
					osMessagePut(AudioVolQueueHandle, DB_VCU_Speed, osWaitForever);
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
	const TickType_t tick1000ms = pdMS_TO_TICKS(1000);
	extern switch_timer_t DB_VCU_Switch_Timer[];
	extern switch_t DB_VCU_Switch[];
	extern uint8_t DB_VCU_Switch_Size;
	extern switcher_t DB_HMI_Switcher;
	uint8_t Last_Mode_Drive;
	uint32_t ulNotifiedValue;
	uint8_t i;

	// Read all EXTI state
	for (i = 0; i < DB_VCU_Switch_Size; i++) {
		DB_VCU_Switch[i].state = HAL_GPIO_ReadPin(DB_VCU_Switch[i].port, DB_VCU_Switch[i].pin);
	}
	// Handle Reverse mode on init
	if (DB_VCU_Switch[IDX_KEY_REVERSE].state) {
		// save previous Drive Mode state
		if (DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] != SWITCH_MODE_DRIVE_R) {
			Last_Mode_Drive = DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE];
		}
		// force state
		DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] = SWITCH_MODE_DRIVE_R;
		// hazard on
		DB_VCU_Switch[IDX_KEY_SEIN_LEFT].state = 1;
		DB_VCU_Switch[IDX_KEY_SEIN_RIGHT].state = 1;
	}

	/* Infinite loop */
	for (;;) {
		xTaskNotifyStateClear(NULL);
		xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
		// handle bounce effect
		osDelay(50);

		// Read all (to handle multiple switch change at the same time)
		for (i = 0; i < DB_VCU_Switch_Size; i++) {
			DB_VCU_Switch[i].state = HAL_GPIO_ReadPin(DB_VCU_Switch[i].port, DB_VCU_Switch[i].pin);

			// handle select & set: timer
			if (i == IDX_KEY_SELECT || i == IDX_KEY_SET) {
				// reset SET timer
				DB_VCU_Switch_Timer[i].time = 0;

				// next job
				if (DB_VCU_Switch[i].state) {
					if (i == IDX_KEY_SELECT || (i == IDX_KEY_SET && DB_HMI_Switcher.listening)) {
						// start timer if not running
						if (!DB_VCU_Switch_Timer[i].running) {
							// set flag
							DB_VCU_Switch_Timer[i].running = 1;
							// start timer for SET
							DB_VCU_Switch_Timer[i].start = osKernelSysTick();
						}
					}
					// reverse it
					DB_VCU_Switch[i].state = 0;
				} else {
					// stop timer if running
					if (DB_VCU_Switch_Timer[i].running) {
						// set flag
						DB_VCU_Switch_Timer[i].running = 0;
						// stop SET
						DB_VCU_Switch_Timer[i].time = (uint8_t) ((osKernelSysTick()
								- DB_VCU_Switch_Timer[i].start) / tick1000ms);
						// reverse it
						DB_VCU_Switch[i].state = 1;
					}
				}
			}
		}

		// Only handle Select & Set when in non-reverse mode
		if (DB_VCU_Switch[IDX_KEY_REVERSE].state) {
			// save previous Drive Mode state
			if (DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] != SWITCH_MODE_DRIVE_R) {
				Last_Mode_Drive = DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE];
			}
			// force state
			DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] = SWITCH_MODE_DRIVE_R;
			// hazard on
			DB_VCU_Switch[IDX_KEY_SEIN_LEFT].state = 1;
			DB_VCU_Switch[IDX_KEY_SEIN_RIGHT].state = 1;
		} else {
			// restore previous Drive Mode
			if (DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] == SWITCH_MODE_DRIVE_R) {
				DB_HMI_Switcher.mode_sub[SWITCH_MODE_DRIVE] = Last_Mode_Drive;
			}

			// handle Select & Set
			if (DB_VCU_Switch[IDX_KEY_SELECT].state || DB_VCU_Switch[IDX_KEY_SET].state) {
				// handle select key
				if (DB_VCU_Switch[IDX_KEY_SELECT].state) {
					if (DB_HMI_Switcher.listening) {
						// change mode position
						if (DB_HMI_Switcher.mode == SWITCH_MODE_MAX) {
							DB_HMI_Switcher.mode = 0;
						} else {
							DB_HMI_Switcher.mode++;
						}
					}
					// Listening on option
					DB_HMI_Switcher.listening = 1;

				} else if (DB_VCU_Switch[IDX_KEY_SET].state) {
					// handle set key
					if (DB_HMI_Switcher.listening
							|| (DB_VCU_Switch_Timer[IDX_KEY_SET].time >= 3 && DB_HMI_Switcher.mode == SWITCH_MODE_TRIP)) {
						// handle reset only if push more than n sec, and in trip mode
						if (!DB_HMI_Switcher.listening) {
							// reset value
							DB_HMI_Switcher.mode_sub_trip[DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]] = 0;
						} else {
							// if less than n sec
							if (DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode] == DB_HMI_Switcher.mode_sub_max[DB_HMI_Switcher.mode]) {
								DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode] = 0;
							} else {
								DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]++;
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
	const TickType_t tick5000ms = pdMS_TO_TICKS(5000);
	TickType_t xLastWakeTime;

	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {

		// Periodic interval
		vTaskDelayUntil(&xLastWakeTime, tick5000ms);
	}
	/* USER CODE END StartGeneralTask */
}

/* CallbackTimerCAN function */
void CallbackTimerCAN(void const *argument)
{
	/* USER CODE BEGIN CallbackTimerCAN */
	// Send CAN data
	CANBUS_VCU_Switch();
	CANBUS_VCU_RTC();
	CANBUS_VCU_Select_Set();
	CANBUS_VCU_Trip_Mode();
	/* USER CODE END CallbackTimerCAN */
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
