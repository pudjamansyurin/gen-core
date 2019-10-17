/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY_SELECT_Pin GPIO_PIN_2
#define KEY_SELECT_GPIO_Port GPIOE
#define KEY_SELECT_EXTI_IRQn EXTI2_IRQn
#define KEY_SET_Pin GPIO_PIN_3
#define KEY_SET_GPIO_Port GPIOE
#define KEY_SET_EXTI_IRQn EXTI3_IRQn
#define KEY_MIRROR_Pin GPIO_PIN_4
#define KEY_MIRROR_GPIO_Port GPIOE
#define KEY_MIRROR_EXTI_IRQn EXTI4_IRQn
#define KEY_LAMP_Pin GPIO_PIN_5
#define KEY_LAMP_GPIO_Port GPIOE
#define KEY_LAMP_EXTI_IRQn EXTI9_5_IRQn
#define KEY_ABS_Pin GPIO_PIN_6
#define KEY_ABS_GPIO_Port GPIOE
#define KEY_ABS_EXTI_IRQn EXTI9_5_IRQn
#define PC14_OSC32_IN_Pin GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define PC15_OSC32_OUT_Pin GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define PH0_OSC_IN_Pin GPIO_PIN_0
#define PH0_OSC_IN_GPIO_Port GPIOH
#define PH1_OSC_OUT_Pin GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_Port GPIOH
#define UBLOX_PWR_Pin GPIO_PIN_1
#define UBLOX_PWR_GPIO_Port GPIOC
#define FINGER_PWR_Pin GPIO_PIN_2
#define FINGER_PWR_GPIO_Port GPIOC
#define I2S3_WS_Pin GPIO_PIN_4
#define I2S3_WS_GPIO_Port GPIOA
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define FINGER_IRQ_Pin GPIO_PIN_7
#define FINGER_IRQ_GPIO_Port GPIOE
#define FINGER_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define NRF24_CE_Pin GPIO_PIN_13
#define NRF24_CE_GPIO_Port GPIOE
#define NRF24_IRQ_Pin GPIO_PIN_15
#define NRF24_IRQ_GPIO_Port GPIOE
#define NRF24_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define SIMCOM_RST_Pin GPIO_PIN_14
#define SIMCOM_RST_GPIO_Port GPIOB
#define SIMCOM_PWR_Pin GPIO_PIN_10
#define SIMCOM_PWR_GPIO_Port GPIOD
#define LD4_Pin GPIO_PIN_12
#define LD4_GPIO_Port GPIOD
#define LD3_Pin GPIO_PIN_13
#define LD3_GPIO_Port GPIOD
#define LD5_Pin GPIO_PIN_14
#define LD5_GPIO_Port GPIOD
#define LD6_Pin GPIO_PIN_15
#define LD6_GPIO_Port GPIOD
#define MEMS_PWR_Pin GPIO_PIN_6
#define MEMS_PWR_GPIO_Port GPIOC
#define I2S3_MCK_Pin GPIO_PIN_7
#define I2S3_MCK_GPIO_Port GPIOC
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define NRF24_CSN_Pin GPIO_PIN_15
#define NRF24_CSN_GPIO_Port GPIOA
#define I2S3_SCK_Pin GPIO_PIN_10
#define I2S3_SCK_GPIO_Port GPIOC
#define I2S3_SD_Pin GPIO_PIN_12
#define I2S3_SD_GPIO_Port GPIOC
#define Audio_RST_Pin GPIO_PIN_4
#define Audio_RST_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define Audio_SCL_Pin GPIO_PIN_6
#define Audio_SCL_GPIO_Port GPIOB
#define Audio_SDA_Pin GPIO_PIN_9
#define Audio_SDA_GPIO_Port GPIOB
#define KEY_SEIN_L_Pin GPIO_PIN_0
#define KEY_SEIN_L_GPIO_Port GPIOE
#define KEY_SEIN_L_EXTI_IRQn EXTI0_IRQn
#define KEY_SEIN_R_Pin GPIO_PIN_1
#define KEY_SEIN_R_GPIO_Port GPIOE
#define KEY_SEIN_R_EXTI_IRQn EXTI1_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
