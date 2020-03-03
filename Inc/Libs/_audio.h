/**
 ******************************************************************************
 * @file    stm32f4_discovery_audio.h
 * @author  MCD Application Team
 * @brief   This file contains the common defines and functions prototypes for
 *          stm32f4_discovery_audio.c driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4_DISCOVERY_AUDIO_H
#define __STM32F4_DISCOVERY_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Include audio component Driver */
#include "_cs43l22.h"
#include "_config.h"

/*############################### AUDIO ######################################*/
#define AUDIO_I2C_ADDRESS               0x94
#define AUDIO_RESET_PIN                 INT_AUDIO_RST_Pin
#define AUDIO_RESET_GPIO                INT_AUDIO_RST_GPIO_Port

/* I2S peripheral configuration defines */
#define I2S                            	SPI3
#define DMA_MAX_SZE                     0xFFFF
/*------------------------------------------------------------------------------
 CONFIGURATION: Audio Driver Configuration parameters
 ------------------------------------------------------------------------------*/
#define AUDIODATA_SIZE                  2   /* 16-bits audio data size */

/* Audio status definition */
#define AUDIO_OK                        0
#define AUDIO_ERROR                     1
#define AUDIO_TIMEOUT                   2

/*------------------------------------------------------------------------------
 OPTIONAL Configuration defines parameters
 ------------------------------------------------------------------------------*/

/* Defines for the Audio playing process */
#define DMA_MAX(_X_)                		(((_X_) <= DMA_MAX_SZE)? (_X_):DMA_MAX_SZE)

void WaveInit(void);
void WavePlay(void);
void WaveBeepPlay(uint8_t Frequency, uint16_t TimeMS);
void WaveBeepStop(void);

uint8_t AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
uint8_t AUDIO_OUT_Play(uint16_t *pBuffer, uint32_t Size);
void AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);
uint8_t AUDIO_OUT_Pause(void);
uint8_t AUDIO_OUT_Resume(void);
uint8_t AUDIO_OUT_Stop(uint32_t Option);
uint8_t AUDIO_OUT_SetVolume(uint8_t Volume);
void AUDIO_OUT_SetFrequency(uint32_t AudioFreq);
uint8_t AUDIO_OUT_SetMute(uint32_t Cmd);
uint8_t AUDIO_OUT_SetOutputMode(uint8_t Output);

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function is called when the requested data has been completely transferred. */
void AUDIO_OUT_TransferComplete_CallBack(void);

/* This function is called when half of the requested buffer has been transferred. */
void AUDIO_OUT_HalfTransfer_CallBack(void);

/* This function is called when an Interrupt due to transfer error on or peripheral
 error occurs. */
void AUDIO_OUT_Error_CallBack(void);

/* These function can be modified in case the current settings (e.g. DMA stream)
 need to be changed for specific application needs */
void AUDIO_OUT_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq, void *Params);
void AUDIO_OUT_MspInit(I2S_HandleTypeDef *hi2s, void *Params);
void AUDIO_OUT_MspDeInit(I2S_HandleTypeDef *hi2s, void *Params);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4_DISCOVERY_AUDIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
