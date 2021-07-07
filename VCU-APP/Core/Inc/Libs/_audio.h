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

/* Define to prevent recursive inclusion
 * --------------------------------------------*/
#ifndef INC_LIBS__AUDIO_H_
#define INC_LIBS__AUDIO_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_cs43l22.h"

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint32_t tick;
  uint8_t active;
  uint8_t mute;
  uint8_t volume;
} audio_data_t;

/* Public functions prototype
 * --------------------------------------------*/
audio_data_t AUDIO_GetData(void);
uint8_t AUDIO_Init(void);
void AUDIO_DeInit(void);
uint8_t AUDIO_Probe(void);
void AUDIO_Refresh(void);
void AUDIO_Flush(void);
uint8_t AUDIO_Play(void);
void AUDIO_BeepPlay(uint8_t Frequency, uint16_t TimeMS);
void AUDIO_BeepStop(void);
uint8_t AUDIO_Mute(uint8_t mute);

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

void AUDIO_OUT_TransferComplete_CallBack(void);
void AUDIO_OUT_HalfTransfer_CallBack(void);
void AUDIO_OUT_Error_CallBack(void);

void AUDIO_OUT_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq,
                           void *Params);

#endif /* INC_LIBS__AUDIO_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
