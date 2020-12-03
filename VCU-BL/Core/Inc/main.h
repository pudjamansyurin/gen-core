/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define SYS_OSC_IN_Pin GPIO_PIN_0
#define SYS_OSC_IN_GPIO_Port GPIOH
#define SYS_OSC_OUT_Pin GPIO_PIN_1
#define SYS_OSC_OUT_GPIO_Port GPIOH
#define INT_NET_PWR_Pin GPIO_PIN_0
#define INT_NET_PWR_GPIO_Port GPIOC
#define EXT_HMI1_PWR_Pin GPIO_PIN_4
#define EXT_HMI1_PWR_GPIO_Port GPIOC
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define EXT_KNOB_IRQ_Pin GPIO_PIN_8
#define EXT_KNOB_IRQ_GPIO_Port GPIOE
#define EXT_KNOB_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define INT_EEPROM_SCL_Pin GPIO_PIN_10
#define INT_EEPROM_SCL_GPIO_Port GPIOB
#define INT_NET_RST_Pin GPIO_PIN_14
#define INT_NET_RST_GPIO_Port GPIOB
#define INT_NET_DTR_Pin GPIO_PIN_15
#define INT_NET_DTR_GPIO_Port GPIOB
#define SYS_LED_Pin GPIO_PIN_13
#define SYS_LED_GPIO_Port GPIOD
#define INT_NET_TX_Pin GPIO_PIN_9
#define INT_NET_TX_GPIO_Port GPIOA
#define INT_NET_RX_Pin GPIO_PIN_10
#define INT_NET_RX_GPIO_Port GPIOA
#define SYS_SWDIO_Pin GPIO_PIN_13
#define SYS_SWDIO_GPIO_Port GPIOA
#define SYS_SWCLK_Pin GPIO_PIN_14
#define SYS_SWCLK_GPIO_Port GPIOA
#define INT_CAN_PWR_Pin GPIO_PIN_3
#define INT_CAN_PWR_GPIO_Port GPIOD
#define SYS_SWO_Pin GPIO_PIN_3
#define SYS_SWO_GPIO_Port GPIOB
#define INT_EEPROM_SDA_Pin GPIO_PIN_9
#define INT_EEPROM_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
