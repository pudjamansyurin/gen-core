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
 * Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *notice, this list of conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef AUDIO_H_
#define AUDIO_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_cs43l22.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define AUDIO_TIMEOUT (uint16_t) 5000 // in ms
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_I2C_ADDRESS 0x94

/* I2S peripheral configuration defines */
#define DMA_MAX_SZE 0xFFFF
#define AUDIO_DATA_SZ 2 /* 16-bits audio data size */

/* Audio status definition */
#define AUDIO_OK 0
#define AUDIO_ERROR 1

/* Exported macro functions
 * ---------------------------------------------------*/
#define DMA_MAX(_X_) (((_X_) <= DMA_MAX_SZE) ? (_X_) : DMA_MAX_SZE)

/* Structs
 * --------------------------------------------------------------------*/
typedef struct {
	uint8_t active;
	uint8_t mute;
	uint8_t volume;
	struct {
		uint16_t played;
		uint32_t remaining;
	} size;
} audio_data_t;

typedef struct {
	audio_data_t d;
	I2S_HandleTypeDef *pi2s;
} audio_t;

/* Exported variables
 * ----------------------------------------------------------*/
extern audio_t AUDIO;

/* Public functions prototype
 * -------------------------------------------------*/
uint8_t AUDIO_Init(void);
void AUDIO_DeInit(void);
void AUDIO_Refresh(void);
void AUDIO_Play(void);
void AUDIO_BeepPlay(uint8_t Frequency, uint16_t TimeMS);
void AUDIO_BeepStop(void);
uint8_t AUDIO_OUT_Pause(void);
uint8_t AUDIO_OUT_Resume(void);
uint8_t AUDIO_OUT_Stop(uint32_t Option);
uint8_t AUDIO_OUT_SetVolume(uint8_t Volume);
void AUDIO_OUT_SetFrequency(uint32_t AudioFreq);
uint8_t AUDIO_OUT_SetOutputMode(uint8_t Output);
uint8_t AUDIO_OUT_SetVolume(uint8_t Volume);
uint8_t AUDIO_OUT_SetMute(uint32_t Cmd);
void AUDIO_OUT_MspInit(I2S_HandleTypeDef *hi2s, void *Params);
void AUDIO_OUT_MspDeInit(I2S_HandleTypeDef *hi2s, void *Params);
void AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);
/* User Callbacks: user has to implement these functions in his code if they are
 * needed. */
/* This function is called when the requested data has been completely
 * transferred. */
void AUDIO_OUT_TransferComplete_CallBack(void);
/* This function is called when half of the requested buffer has been
 * transferred. */
void AUDIO_OUT_HalfTransfer_CallBack(void);
/* This function is called when an Interrupt due to transfer error on or
 peripheral error occurs. */
void AUDIO_OUT_Error_CallBack(void);
/* These function can be modified in case the current settings (e.g. DMA stream)
 need to be changed for specific application needs */
void AUDIO_OUT_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq,
		void *Params);

#endif /* AUDIO_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
