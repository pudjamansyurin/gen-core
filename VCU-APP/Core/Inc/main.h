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
#define EXT_HBAR_SELECT_Pin GPIO_PIN_2
#define EXT_HBAR_SELECT_GPIO_Port GPIOE
#define EXT_HBAR_SELECT_EXTI_IRQn EXTI2_IRQn
#define EXT_HBAR_SET_Pin GPIO_PIN_3
#define EXT_HBAR_SET_GPIO_Port GPIOE
#define EXT_HBAR_SET_EXTI_IRQn EXTI3_IRQn
#define EXT_HBAR_REVERSE_Pin GPIO_PIN_5
#define EXT_HBAR_REVERSE_GPIO_Port GPIOE
#define EXT_HBAR_REVERSE_EXTI_IRQn EXTI9_5_IRQn
#define EXT_ABS_IRQ_Pin GPIO_PIN_6
#define EXT_ABS_IRQ_GPIO_Port GPIOE
#define EXT_ABS_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define INT_REMOTE_CE_Pin GPIO_PIN_13
#define INT_REMOTE_CE_GPIO_Port GPIOC
#define SYS_OSC32_IN_Pin GPIO_PIN_14
#define SYS_OSC32_IN_GPIO_Port GPIOC
#define SYS_OSC32_OUT_Pin GPIO_PIN_15
#define SYS_OSC32_OUT_GPIO_Port GPIOC
#define SYS_OSC_IN_Pin GPIO_PIN_0
#define SYS_OSC_IN_GPIO_Port GPIOH
#define SYS_OSC_OUT_Pin GPIO_PIN_1
#define SYS_OSC_OUT_GPIO_Port GPIOH
#define INT_NET_PWR_Pin GPIO_PIN_0
#define INT_NET_PWR_GPIO_Port GPIOC
#define INT_GPS_PWR_Pin GPIO_PIN_1
#define INT_GPS_PWR_GPIO_Port GPIOC
#define EXT_FINGER_MCU_PWR_Pin GPIO_PIN_2
#define EXT_FINGER_MCU_PWR_GPIO_Port GPIOC
#define EXT_FINGER_SENSING_PWR_Pin GPIO_PIN_3
#define EXT_FINGER_SENSING_PWR_GPIO_Port GPIOC
#define EXT_FINGER_TX_Pin GPIO_PIN_0
#define EXT_FINGER_TX_GPIO_Port GPIOA
#define EXT_FINGER_RX_Pin GPIO_PIN_1
#define EXT_FINGER_RX_GPIO_Port GPIOA
#define INT_GPS_TX_Pin GPIO_PIN_2
#define INT_GPS_TX_GPIO_Port GPIOA
#define INT_GPS_RX_Pin GPIO_PIN_3
#define INT_GPS_RX_GPIO_Port GPIOA
#define INT_AUDIO_LRCK_Pin GPIO_PIN_4
#define INT_AUDIO_LRCK_GPIO_Port GPIOA
#define INT_REMOTE_SCK_Pin GPIO_PIN_5
#define INT_REMOTE_SCK_GPIO_Port GPIOA
#define INT_REMOTE_MISO_Pin GPIO_PIN_6
#define INT_REMOTE_MISO_GPIO_Port GPIOA
#define INT_REMOTE_MOSI_Pin GPIO_PIN_7
#define INT_REMOTE_MOSI_GPIO_Port GPIOA
#define EXT_HMI1_PWR_Pin GPIO_PIN_4
#define EXT_HMI1_PWR_GPIO_Port GPIOC
#define EXT_HMI2_PWR_Pin GPIO_PIN_5
#define EXT_HMI2_PWR_GPIO_Port GPIOC
#define EXT_SEAT_PWR_Pin GPIO_PIN_0
#define EXT_SEAT_PWR_GPIO_Port GPIOB
#define INT_ADC_VBAT_Pin GPIO_PIN_1
#define INT_ADC_VBAT_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define EXT_FINGER_IRQ_Pin GPIO_PIN_7
#define EXT_FINGER_IRQ_GPIO_Port GPIOE
#define EXT_FINGER_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define EXT_STARTER_IRQ_Pin GPIO_PIN_8
#define EXT_STARTER_IRQ_GPIO_Port GPIOE
#define EXT_STARTER_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define EXT_GPIO_IN1_Pin GPIO_PIN_9
#define EXT_GPIO_IN1_GPIO_Port GPIOE
#define EXT_GPIO_IN1_EXTI_IRQn EXTI9_5_IRQn
#define INT_GYRO_IRQ_Pin GPIO_PIN_11
#define INT_GYRO_IRQ_GPIO_Port GPIOE
#define INT_GYRO_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define EXT_HBAR_LAMP_Pin GPIO_PIN_13
#define EXT_HBAR_LAMP_GPIO_Port GPIOE
#define EXT_HBAR_LAMP_EXTI_IRQn EXTI15_10_IRQn
#define EXT_REG_5V_IRQ_Pin GPIO_PIN_14
#define EXT_REG_5V_IRQ_GPIO_Port GPIOE
#define EXT_REG_5V_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define INT_REMOTE_IRQ_Pin GPIO_PIN_15
#define INT_REMOTE_IRQ_GPIO_Port GPIOE
#define INT_REMOTE_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define INT_EEPROM_SCL_Pin GPIO_PIN_10
#define INT_EEPROM_SCL_GPIO_Port GPIOB
#define INT_NET_RST_Pin GPIO_PIN_14
#define INT_NET_RST_GPIO_Port GPIOB
#define INT_NET_DTR_Pin GPIO_PIN_15
#define INT_NET_DTR_GPIO_Port GPIOB
#define EXT_GPIO_OUT1_Pin GPIO_PIN_8
#define EXT_GPIO_OUT1_GPIO_Port GPIOD
#define EXT_GPIO_OUT2_Pin GPIO_PIN_9
#define EXT_GPIO_OUT2_GPIO_Port GPIOD
#define SYS_LED_Pin GPIO_PIN_13
#define SYS_LED_GPIO_Port GPIOD
#define INT_GPS_SLEEP_Pin GPIO_PIN_6
#define INT_GPS_SLEEP_GPIO_Port GPIOC
#define INT_AUDIO_MCLK_Pin GPIO_PIN_7
#define INT_AUDIO_MCLK_GPIO_Port GPIOC
#define EXT_HORN_PWR_Pin GPIO_PIN_8
#define EXT_HORN_PWR_GPIO_Port GPIOC
#define INT_GYRO_SDA_Pin GPIO_PIN_9
#define INT_GYRO_SDA_GPIO_Port GPIOC
#define INT_GYRO_SCL_Pin GPIO_PIN_8
#define INT_GYRO_SCL_GPIO_Port GPIOA
#define INT_NET_TX_Pin GPIO_PIN_9
#define INT_NET_TX_GPIO_Port GPIOA
#define INT_NET_RX_Pin GPIO_PIN_10
#define INT_NET_RX_GPIO_Port GPIOA
#define SYS_SWDIO_Pin GPIO_PIN_13
#define SYS_SWDIO_GPIO_Port GPIOA
#define SYS_SWCLK_Pin GPIO_PIN_14
#define SYS_SWCLK_GPIO_Port GPIOA
#define INT_REMOTE_CSN_Pin GPIO_PIN_15
#define INT_REMOTE_CSN_GPIO_Port GPIOA
#define INT_AUDIO_SCLK_Pin GPIO_PIN_10
#define INT_AUDIO_SCLK_GPIO_Port GPIOC
#define INT_AUDIO_PWR_Pin GPIO_PIN_11
#define INT_AUDIO_PWR_GPIO_Port GPIOC
#define INT_AUDIO_SDIN_Pin GPIO_PIN_12
#define INT_AUDIO_SDIN_GPIO_Port GPIOC
#define INT_CAN_RX_Pin GPIO_PIN_0
#define INT_CAN_RX_GPIO_Port GPIOD
#define INT_CAN_TX_Pin GPIO_PIN_1
#define INT_CAN_TX_GPIO_Port GPIOD
#define INT_CAN_PWR_Pin GPIO_PIN_3
#define INT_CAN_PWR_GPIO_Port GPIOD
#define INT_AUDIO_RST_Pin GPIO_PIN_4
#define INT_AUDIO_RST_GPIO_Port GPIOD
#define EXT_BMS_WAKEUP_Pin GPIO_PIN_5
#define EXT_BMS_WAKEUP_GPIO_Port GPIOD
#define EXT_BMS_FAN_PWR_Pin GPIO_PIN_6
#define EXT_BMS_FAN_PWR_GPIO_Port GPIOD
#define SYS_SWO_Pin GPIO_PIN_3
#define SYS_SWO_GPIO_Port GPIOB
#define INT_GYRO_PWR_Pin GPIO_PIN_4
#define INT_GYRO_PWR_GPIO_Port GPIOB
#define INT_REMOTE_PWR_Pin GPIO_PIN_5
#define INT_REMOTE_PWR_GPIO_Port GPIOB
#define INT_AUDIO_SCL_Pin GPIO_PIN_6
#define INT_AUDIO_SCL_GPIO_Port GPIOB
#define INT_AUDIO_SDA_Pin GPIO_PIN_7
#define INT_AUDIO_SDA_GPIO_Port GPIOB
#define INT_BUZZER_PWM_Pin GPIO_PIN_8
#define INT_BUZZER_PWM_GPIO_Port GPIOB
#define INT_EEPROM_SDA_Pin GPIO_PIN_9
#define INT_EEPROM_SDA_GPIO_Port GPIOB
#define EXT_HBAR_SEIN_L_Pin GPIO_PIN_0
#define EXT_HBAR_SEIN_L_GPIO_Port GPIOE
#define EXT_HBAR_SEIN_L_EXTI_IRQn EXTI0_IRQn
#define EXT_HBAR_SEIN_R_Pin GPIO_PIN_1
#define EXT_HBAR_SEIN_R_GPIO_Port GPIOE
#define EXT_HBAR_SEIN_R_EXTI_IRQn EXTI1_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
