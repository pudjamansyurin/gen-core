/**
 ******************************************************************************
 * @file    cs43l22.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the cs43l22.c
 *driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#ifndef INC_DRIVERS__CS43L22_H_
#define INC_DRIVERS__CS43L22_H_

/* Includes
 * --------------------------------------------*/
#include "App/common.h"

/* Exported enums
 * --------------------------------------------*/
/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Codec output DEVICE */
typedef enum {
  CS_OUT_DEV_SPEAKER = 1,
  CS_OUT_DEV_HEADPHONE,
  CS_OUT_DEV_BOTH,
  CS_OUT_DEV_AUTO,
} CS_OUT_DEV;

/* Codec POWER DOWN modes */
typedef enum { CS_CODEC_PDWN_HW = 1, CS_CODEC_PDWN_SW } CS_CODEC_PDWN;

typedef enum { CS_AUDIO_MUTE_OFF, CS_AUDIO_MUTE_ON } CS_AUDIO_MUTE;

typedef enum {
  CS_BEEP_MODE_OFF,
  CS_BEEP_MODE_SINGLE,
  CS_BEEP_MODE_MULTIPLE,
  CS_BEEP_MODE_CONTINUOUS,
} CS_BEEP_MODE;

typedef enum {
  CS_BEEP_MIX_ON,
  CS_BEEP_MIX_OFF,
} CS_BEEP_MIX;

typedef enum {
  CS_BEEP_FREQ_260_HZ,
  CS_BEEP_FREQ_521_HZ,
  CS_BEEP_FREQ_585_HZ,
  CS_BEEP_FREQ_666_HZ,
  CS_BEEP_FREQ_705_HZ,
  CS_BEEP_FREQ_774_HZ,
  CS_BEEP_FREQ_888_HZ,
  CS_BEEP_FREQ_1000_HZ,
  CS_BEEP_FREQ_1043_HZ,
  CS_BEEP_FREQ_1200_HZ,
  CS_BEEP_FREQ_1333_HZ,
  CS_BEEP_FREQ_1411_HZ,
  CS_BEEP_FREQ_1600_HZ,
  CS_BEEP_FREQ_1714_HZ,
  CS_BEEP_FREQ_2000_HZ,
  CS_BEEP_FREQ_2181_HZ,
} CS_BEEP_FREQ;

/* Public functions prototype
 * --------------------------------------------*/
uint32_t cs43l22_Init(uint16_t OutputDevice, uint8_t Vol, uint32_t AudioFreq);
void cs43l22_DeInit(void);
uint8_t cs43l22_Probe(void);
uint32_t cs43l22_ReadID(void);
uint32_t cs43l22_Play(void);
uint32_t cs43l22_Pause(void);
uint32_t cs43l22_Resume(void);
uint32_t cs43l22_Stop(uint32_t Cmd);
uint32_t cs43l22_SetVolume(uint8_t Vol);
uint32_t cs43l22_SetFrequency(uint32_t AudioFreq);
uint32_t cs43l22_SetMute(uint32_t Cmd);
uint32_t cs43l22_SetOutputMode(uint8_t Output);
uint32_t cs43l22_Reset(void);
uint32_t cs43l22_SetBeep(uint8_t Frequency, uint8_t OnTime, uint8_t OffTime);
uint32_t cs43l22_Beep(uint8_t Mode, uint8_t Mix);

#endif /* INC_DRIVERS__CS43L22_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
