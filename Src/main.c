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
#include "_eeprom.h"
#include "_nrf24l01.h"
#include "_canbus.h"
#include "_database.h"
#include "_rtc.h"
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
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart3_rx;

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
osMessageQId AudioVolQueueHandle;
osTimerId TimerCANHandle;
osMutexId AudioBeepMutexHandle;
osMutexId SwvMutexHandle;
osMutexId CanTxMutexHandle;
osMutexId SimcomRecMutexHandle;
osMutexId FingerRecMutexHandle;
/* USER CODE BEGIN PV */
osMailQId GpsMailHandle;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
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
	MX_USART3_UART_Init();
	MX_USART2_UART_Init();
	MX_UART4_Init();
	MX_SPI1_Init();
	MX_RTC_Init();
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
	osTimerStart(TimerCANHandle, 500);
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* definition and creation of AudioVolQueue */
	osMessageQDef(AudioVolQueue, 1, uint8_t);
	AudioVolQueueHandle = osMessageCreate(osMessageQ(AudioVolQueue), NULL);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	osMailQDef(GpsMail, 1, gps_t);
	GpsMailHandle = osMailCreate(osMailQ(GpsMail), NULL);

	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of IotTask */
	osThreadDef(IotTask, StartIotTask, osPriorityNormal, 0, 512);
	IotTaskHandle = osThreadCreate(osThread(IotTask), NULL);

	/* definition and creation of GyroTask */
	osThreadDef(GyroTask, StartGyroTask, osPriorityNormal, 0, 512);
	GyroTaskHandle = osThreadCreate(osThread(GyroTask), NULL);

	/* definition and creation of CommandTask */
	osThreadDef(CommandTask, StartCommandTask, osPriorityNormal, 0, 256);
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
	osThreadDef(KeylessTask, StartKeylessTask, osPriorityNormal, 0, 256);
	KeylessTaskHandle = osThreadCreate(osThread(KeylessTask), NULL);

	/* definition and creation of ReporterTask */
	osThreadDef(ReporterTask, StartReporterTask, osPriorityNormal, 0, 512);
	ReporterTaskHandle = osThreadCreate(osThread(ReporterTask), NULL);

	/* definition and creation of CanRxTask */
	osThreadDef(CanRxTask, StartCanRxTask, osPriorityAboveNormal, 0, 128);
	CanRxTaskHandle = osThreadCreate(osThread(CanRxTask), NULL);

	/* definition and creation of SwitchTask */
	osThreadDef(SwitchTask, StartSwitchTask, osPriorityNormal, 0, 128);
	SwitchTaskHandle = osThreadCreate(osThread(SwitchTask), NULL);

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
	PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
	PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
			{
		Error_Handler();
	}
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
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

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
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
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
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void)
{

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	//  huart3.Init.BaudRate = 115200;
	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 9600;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK)
			{
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	/* DMA1_Stream2_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
	/* DMA1_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	/* DMA1_Stream7_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

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
	HAL_GPIO_WritePin(GPIOC, NRF24_PWR_Pin | UBLOX_PWR_Pin | SPEEDO_PWR_Pin | MIRROR_PWR_Pin
			| MEMS_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(FINGER_PWR_GPIO_Port, FINGER_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, MIRROR_LIGHT_Pin | COOLING_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SIMCOM_RST_GPIO_Port, SIMCOM_RST_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SIMCOM_PWR_GPIO_Port, SIMCOM_PWR_Pin, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, LD4_Pin | LD3_Pin | LD5_Pin | LD6_Pin
			| Audio_RST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(NRF24_CSN_GPIO_Port, NRF24_CSN_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : KEY_SELECT_Pin KEY_SET_Pin KEY_MIRROR_Pin KEY_REVERSE_Pin
	 KEY_ABS_Pin KEY_SEIN_L_Pin KEY_SEIN_R_Pin */
	GPIO_InitStruct.Pin = KEY_SELECT_Pin | KEY_SET_Pin | KEY_MIRROR_Pin | KEY_REVERSE_Pin
			| KEY_ABS_Pin | KEY_SEIN_L_Pin | KEY_SEIN_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : BMS_IRQ_Pin */
	GPIO_InitStruct.Pin = BMS_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(BMS_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : NRF24_PWR_Pin UBLOX_PWR_Pin FINGER_PWR_Pin SPEEDO_PWR_Pin
	 MIRROR_PWR_Pin MEMS_PWR_Pin */
	GPIO_InitStruct.Pin = NRF24_PWR_Pin | UBLOX_PWR_Pin | FINGER_PWR_Pin | SPEEDO_PWR_Pin
			| MIRROR_PWR_Pin | MEMS_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : MIRROR_LIGHT_Pin COOLING_PWR_Pin SIMCOM_RST_Pin */
	GPIO_InitStruct.Pin = MIRROR_LIGHT_Pin | COOLING_PWR_Pin | SIMCOM_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : BOOT1_Pin */
	GPIO_InitStruct.Pin = BOOT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : FINGER_IRQ_Pin */
	GPIO_InitStruct.Pin = FINGER_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(FINGER_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF24_CE_Pin */
	GPIO_InitStruct.Pin = NRF24_CE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(NRF24_CE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF24_IRQ_Pin */
	GPIO_InitStruct.Pin = NRF24_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(NRF24_IRQ_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SIMCOM_PWR_Pin LD4_Pin LD3_Pin LD5_Pin
	 LD6_Pin Audio_RST_Pin */
	GPIO_InitStruct.Pin = SIMCOM_PWR_Pin | LD4_Pin | LD3_Pin | LD5_Pin
			| LD6_Pin | Audio_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : NRF24_CSN_Pin */
	GPIO_InitStruct.Pin = NRF24_CSN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(NRF24_CSN_GPIO_Port, &GPIO_InitStruct);

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
	// nothing yet to do
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	extern nrf24l01 nrf;

	if (osKernelRunning()) {
		switch (GPIO_Pin) {
		case NRF24_IRQ_Pin:
			nrf_irq_handler(&nrf);
			break;
		case FINGER_IRQ_Pin:
			xTaskNotifyFromISR(FingerTaskHandle, EVENT_FINGER_PLACED, eSetBits,
					&xHigherPriorityTaskWoken);
			break;
		case BMS_IRQ_Pin:
			//NOTED do something with me
			break;
		default:
			xTaskNotifyFromISR(SwitchTaskHandle, (uint32_t ) GPIO_Pin, eSetBits,
					&xHigherPriorityTaskWoken);
			break;
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
	uint32_t ulNotifiedValue;
	// Start simcom module
	SIMCOM_DMA_Init();
	Simcom_Init(0);
	/* Infinite loop */
	for (;;) {
		// get event data
		xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

		// check every event
		if (ulNotifiedValue & EVENT_IOT_SEND_REPORT) {
			// Send payload
			if (!Simcom_Send_Payload()) {
				// If signal lost, force restart directly
				if (!Simcom_Signal_Locked(1)) {
					// FIXME remove my warning
					//					WaveBeepPlay(BEEP_FREQ_2000_HZ, 5000);
					BSP_Led_Disco(5000);
					// restart module
					Simcom_Init(0);
				}
			}
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
	mems_t mems_calibration;
	mems_decision_t mems_decision;
	SD_MPU6050 mpu;
	/* MPU6050 Initialization*/
	MEMS_Init(&hi2c3, &mpu);
	// Set calibrator
	mems_calibration = MEMS_Average(&hi2c3, &mpu, NULL, 500);
	// Give success indicator
	WaveBeepPlay(BEEP_FREQ_2000_HZ, 50);
	/* Infinite loop */
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
		osDelay(1);
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
	uint32_t ulNotifiedValue;
	BaseType_t xResult;
	command_t command;
	char response[100];
	int p;
	uint8_t newCommand = 0;
	uint32_t val;
	/* Infinite loop */
	for (;;) {
		// check if command arrived from IOT Task
		xResult = xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, 0);
		if (xResult == pdTRUE) {
			if ((ulNotifiedValue & EVENT_COMMAND_ARRIVED)) {
				newCommand = 1;
			}
		}

		if (Simcom_Check_Command() || newCommand) {
			newCommand = 0;
			// read the command & execute
			if (Simcom_Get_Command(&command)) {
				// generic command response
				sprintf(response, "%s executed.", command.cmd);

				// BSP Led configuration
				if (strstr(command.var, "LED") != NULL) {
					val = atoi(command.val);
					if (strcmp(command.var, "LED1") == 0) {
						HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, val);
					} else if (strcmp(command.var, "LED2") == 0) {
						HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, val);
					}
				}

				// RTC configuration
				else if (strcmp(command.var, "RTC") == 0) {
					RTC_Write(command.val);
				}

				// Odometer configuration
				else if (strcmp(command.var, "ODOM") == 0) {
					val = atoi(command.val);
					Reporter_Set_Odometer(val);
				}

				// Information detail
				else if (strcmp(command.var, "INFO") == 0) {
					sprintf(response, "HUB v.1.0\nGEN Indonesia @ 2019\n");
				}

				// Audio configuration
				else if (strstr(command.var, "AUDIO_") != NULL) {
					if (strcmp(command.var, "AUDIO_BEEP") == 0) {
						xTaskNotify(AudioTaskHandle, EVENT_AUDIO_BEEP, eSetBits);
					}

					else {
						val = atoi(command.val);

						if (strcmp(command.var, "AUDIO_MUTE") == 0) {
							if (val) {
								xTaskNotify(AudioTaskHandle, EVENT_AUDIO_MUTE_ON, eSetBits);
							} else {
								xTaskNotify(AudioTaskHandle, EVENT_AUDIO_MUTE_OFF, eSetBits);
							}
						}

						else if (strcmp(command.var, "AUDIO_VOL") == 0) {
							osMessagePut(AudioVolQueueHandle, (uint8_t) val, osWaitForever);
						}
					}

				}

				// Finger print configuration
				else if (strstr(command.var, "FINGER_") != NULL) {
					val = atoi(command.val);

					if (strcmp(command.var, "FINGER_ADD") == 0) {
						p = Finger_Enroll(val);
					}

					else if (strcmp(command.var, "FINGER_DELETE") == 0) {
						p = Finger_Delete_ID(val);
					}

					else if (strcmp(command.var, "FINGER_RESET") == 0) {
						p = Finger_Empty_Database();
					}

					sprintf(response, "%s", command.cmd);
					if (p == FINGERPRINT_OK) {
						sprintf(response, "%s OK", response);
					} else {
						sprintf(response, "%s ERROR", response);
					}
				}

				else {
					sprintf(response, "%s not found.", command.cmd);
				}

				// send confirmation
				Simcom_To_Server(response, strlen(response));
			}
		}
		osDelay(100);
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
	const TickType_t xDelay_ms = pdMS_TO_TICKS(REPORT_INTERVAL*1000);
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
		// hand-over data to IOT_Task
		osMailPut(GpsMailHandle, hgps);

		// Report interval in second
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
				BSP_Led_Write_All(1);
				osDelay(1000);
			}
		}
		BSP_Led_Write_All(0);
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
	uint32_t ulNotifiedValue;
	BaseType_t xResult;
	osEvent evt;
	/* Initialize Wave player (Codec, DMA, I2C) */
	WaveInit();
	// Play wave loop forever, handover to DMA, so CPU is free
	WavePlay();
	/* Infinite loop */
	for (;;) {
		// check if get volume message
		evt = osMessageGet(AudioVolQueueHandle, 0);
		// do this if message arrived
		if (evt.status == osEventMessage) {
			BSP_AUDIO_OUT_SetVolume((uint8_t) evt.value.v);
		}

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
				BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
			}
			if ((ulNotifiedValue & EVENT_AUDIO_MUTE_OFF)) {
				BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
			}
		}

		osDelay(500);
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

	uint8_t payloadSize32 = 32 / 4;
	uint32_t payloadReceived[payloadSize32];
	//	uint8_t i, packetGood;

	// set configuration
	nrf_set_config(&config, payloadReceived, payloadSize32);
	// initialization
	nrf_init(&nrf, &config);
	/* Infinite loop */
	for (;;) {
		SWV_SendStrLn("NRF waiting packet.");
		// receive packet (blocking)
		nrf_receive_packet(&nrf);

		// check received packet
		//		packetGood = 1;
		//		for (i = 0; (i < payloadSize32) && packetGood; i++) {
		//			if (payloadReceived[i] != payloadFinder[i]) {
		//				packetGood = 0;
		//			}
		//		}

		//		if (packetGood) {
		if (payloadReceived[payloadSize32 - 1] == EVENT_KEYLESS_FINDER) {
			WaveBeepPlay(BEEP_FREQ_2000_HZ, 500);
		} else if (payloadReceived[payloadSize32 - 1] == EVENT_KEYLESS_BROADCAST) {
			WaveBeepPlay(BEEP_FREQ_2000_HZ, 100);
		}

		// indicator
		BSP_Led_Toggle_All();
		osDelay(10);
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
	const TickType_t xDelay_ms = pdMS_TO_TICKS(REPORT_INTERVAL*1000);
	TickType_t xLastWakeTime;
	BaseType_t xResult;
	uint32_t ulNotifiedValue;
	osEvent evt;

	char msg[REPORT_MESSAGE_LENGTH];
	report_id_t reportID;
	// reset data to default
	Reporter_Reset();
	/* Infinite loop */
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		// reset message & report
		reportID = REPORT_OK;
		sprintf(msg, "%s", "");

		// get event data
		xResult = xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, 0);

		// do this if events occurred
		if (xResult == pdTRUE) {
			// check every event
			if (ulNotifiedValue & EVENT_REPORTER_CRASH) {
				reportID = REPORT_BIKE_CRASHED;
			}
			if ((ulNotifiedValue & EVENT_REPORTER_FALL)
					&& !(ulNotifiedValue & EVENT_REPORTER_FALL_FIXED)) {
				reportID = REPORT_BIKE_FALLING;
			}
		}

		// set report
		Reporter_Set_Report_ID(reportID);

		// set message
		Reporter_Set_Message(msg);

		// get processed gps data
		evt = osMailGet(GpsMailHandle, 0);
		// Convert Ublox data to Simcom payload compatible
		if (evt.status == osEventMail) {
			Reporter_Convert_GPS(evt.value.p);
		}

		// Set payload (all data)
		// FIXME i need to be stored in array logs.
		Reporter_Set_Payload();

		// Report is ready, do what you want
		xTaskNotify(IotTaskHandle, EVENT_IOT_SEND_REPORT, eSetBits);

		// Report interval in second
		vTaskDelayUntil(&xLastWakeTime, xDelay_ms);
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
	extern uint8_t DB_ECU_Speed;
	extern CAN_Rx RxCan;
	//	uint8_t i;
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
				osMessagePut(AudioVolQueueHandle, DB_ECU_Speed, osWaitForever);
				break;
			default:
				break;
			}

			//			// show this message
			//			SWV_SendStr("ID: ");
			//			SWV_SendHex32(RxCan.RxHeader.StdId);
			//			SWV_SendStr(", Data: ");
			//			for (i = 0; i < RxCan.RxHeader.DLC; i++) {
			//				SWV_SendHex8(RxCan.RxData[i]);
			//			}
			//			SWV_SendStrLn("");
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
	// NOTED add 'cost' to all constant
	const uint64_t tickSecond = osKernelSysTickMicroSec(1000000);
	extern switch_timer_t DB_ECU_Switch_Timer[];
	extern switch_t DB_ECU_Switch[];
	extern uint8_t DB_ECU_Switch_Size;
	extern switcher_t DB_HMI_Switcher;
	// FIXME use me as binary event
	uint32_t ulNotifiedValue;
	uint8_t i;
	// Read all initial state
	for (i = 0; i < DB_ECU_Switch_Size; i++) {
		DB_ECU_Switch[i].state = HAL_GPIO_ReadPin(DB_ECU_Switch[i].port,
				DB_ECU_Switch[i].pin);
	}
	/* Infinite loop */
	for (;;) {
		xTaskNotifyStateClear(NULL);
		xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
		// handle bounce effect
		osDelay(50);

		// Read all (to handle multiple switch change at the same time)
		for (i = 0; i < DB_ECU_Switch_Size; i++) {
			DB_ECU_Switch[i].state = HAL_GPIO_ReadPin(DB_ECU_Switch[i].port,
					DB_ECU_Switch[i].pin);
			// handle select & set: timer
			if (i == IDX_KEY_SELECT || i == IDX_KEY_SET) {
				// reset time
				DB_ECU_Switch_Timer[i].time = 0;
				// next job
				if (DB_ECU_Switch[i].state) {
					if (!DB_ECU_Switch_Timer[i].running) {
						// set flag
						DB_ECU_Switch_Timer[i].running = 1;
						// start timer for select
						DB_ECU_Switch_Timer[i].start = osKernelSysTick();
					}
					// reverse it
					DB_ECU_Switch[i].state = !DB_ECU_Switch[i].state;
				} else {
					if (DB_ECU_Switch_Timer[i].running) {
						// set flag
						DB_ECU_Switch_Timer[i].running = 0;
						// stop timer
						DB_ECU_Switch_Timer[i].time = (uint8_t) ((osKernelSysTick()
								- DB_ECU_Switch_Timer[i].start) / tickSecond);
						// reverse it
						DB_ECU_Switch[i].state = !DB_ECU_Switch[i].state;
					}
				}
			}
		}

		// Only handle Select & Set when in non-reverse mode
		if (!DB_ECU_Switch[IDX_KEY_REVERSE].state) {
			// handle Select & Set
			if (DB_ECU_Switch[IDX_KEY_SELECT].state || DB_ECU_Switch[IDX_KEY_SET].state) {
				// handle select key
				if (DB_ECU_Switch[IDX_KEY_SELECT].state) {
					// change mode position
					if (DB_HMI_Switcher.mode == SWITCH_MODE_MAX) {
						DB_HMI_Switcher.mode = 0;
					} else {
						DB_HMI_Switcher.mode++;
					}
				}

				// handle set key
				if (DB_ECU_Switch[IDX_KEY_SET].state) {
					// handle reset only if push more than 5sec, and in trip mode
					if (DB_ECU_Switch_Timer[IDX_KEY_SET].time > 5
							&& DB_HMI_Switcher.mode == SWITCH_MODE_TRIP) {
						// reset value
						DB_HMI_Switcher.mode_sub_trip[DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]] = 0;
					} else {
						// if less than 5sec
						if (DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]
								== DB_HMI_Switcher.mode_sub_max[DB_HMI_Switcher.mode]) {
							DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode] = 0;
						} else {
							DB_HMI_Switcher.mode_sub[DB_HMI_Switcher.mode]++;
						}
					}
				}
			}
		}

		//		// show state
		//		SWV_SendStrLn("\n============================");
		//		for (i = 0; i < DB_ECU_Switch_Size; i++) {
		//			SWV_SendBuf(DB_ECU_Switch[i].event, strlen(DB_ECU_Switch[i].event));
		//			SWV_SendStr(" = ");
		//			SWV_SendInt(DB_ECU_Switch[i].state);
		//			// show periode
		//			if (i == IDX_KEY_SELECT || i == IDX_KEY_SET) {
		//				SWV_SendStr(" (");
		//				SWV_SendInt(DB_ECU_Switch_Timer[i].time);
		//				SWV_SendStr(")");
		//			}
		//			SWV_SendStrLn("");
		//		}
	}
	/* USER CODE END StartSwitchTask */
}

/* CallbackTimerCAN function */
void CallbackTimerCAN(void const *argument)
{
	/* USER CODE BEGIN CallbackTimerCAN */
	CANBUS_ECU_Switch();
	CANBUS_ECU_RTC();
	CANBUS_ECU_Select_Set();
	CANBUS_ECU_Trip_Mode();

	BSP_Led_Toggle_All();
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
