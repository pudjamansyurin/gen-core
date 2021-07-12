/**
 ******************************************************************************
 * @file    stm32f4_discovery_audio.c
 * @author  MCD Application Team
 * @brief   This file provides the Audio driver for the STM32F4-Discovery
 *          board.
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

/* Includes
 * --------------------------------------------*/
#include "Libs/_audio.h"

#include "i2s.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t AudioRecMutexHandle;
#endif
extern uint32_t SOUND_FREQ, SOUND_SIZE;
extern uint16_t SOUND_SAMPLE[];

/* Private constants
 * --------------------------------------------*/
#define AUDIO_TIMEOUT_MS ((uint16_t)5000)
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_PLL_CNT 8
/* 16-bits audio data size */
#define AUDIO_DATA_SZ 2
#define DMA_MAX_SZE 0xFFFF
/* Audio status definition */
#define AUDIO_OK 0
#define AUDIO_ERROR 1

/* Private macros
 * --------------------------------------------*/
#define DMA_MAX(X) (((X) <= DMA_MAX_SZE) ? (X) : DMA_MAX_SZE)

/* Exported types
 * -------------------------------------------*/
typedef uint32_t audio_pll_t[AUDIO_PLL_CNT];

typedef struct {
  uint16_t played;
  uint32_t remaining;
} audio_size_t;

typedef struct {
  audio_pll_t freq;
  audio_pll_t plln;
  audio_pll_t pllr;
} audio_i2s_t;

typedef struct {
  audio_data_t d;
  audio_size_t sz;
  audio_i2s_t i2s;
  I2S_HandleTypeDef *pi2s;
} audio_t;

/* Private variables
 * --------------------------------------------*/
static audio_t AUDIO = {
    .d = {0},
    .sz = {0},
    .pi2s = &hi2s3,
    .i2s = {
        /* These PLL parameters are valid when the f(VCO clock) = 1Mhz */
        .freq = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000},
        .plln = {256, 429, 213, 429, 426, 271, 258, 344},
        .pllr = {5, 4, 4, 4, 4, 6, 3, 1},
    }};

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t I2S_Init(uint32_t AudioFreq);
static uint8_t AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume,
                              uint32_t AudioFreq);
static void AUDIO_OUT_DeInit(void);
static uint8_t AUDIO_OUT_Play(uint16_t *Buffer, uint32_t Size);
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t AUDIO_Init(void) {
  uint8_t ok;
  uint32_t tick;

  /* Initialize Wave player (Codec, DMA, I2C) */
  lock();
  printf("AUDIO:Init\n");

  tick = tickMs();
  do {
    GATE_AudioReset();
    ok = AUDIO_OUT_Init(CS_OUT_DEV_HEADPHONE, AUDIO.d.volume, SOUND_FREQ) ==
         AUDIO_OK;
    if (!ok) delayMs(500);
  } while (!ok && tickIn(tick, AUDIO_TIMEOUT_MS));
  unlock();

  printf("AUDIO:%s\n", ok ? "OK" : "Error");
  return ok;
}

void AUDIO_DeInit(void) {
  lock();
  AUDIO_Flush();
  AUDIO_OUT_DeInit();
  GATE_AudioShutdown();
  unlock();
}

uint8_t AUDIO_Probe(void) {
  uint8_t ok;

  lock();
  ok = cs43l22_Probe();
  unlock();

  return ok;
}

void AUDIO_Refresh(void) {
  lock();
  AUDIO.d.active = tickIn(AUDIO.d.tick, AUDIO_TIMEOUT_MS);

  if (!AUDIO.d.active) {
    // FIXME: Re-init not recover
    AUDIO_DeInit();
    delayMs(500);
    AUDIO_Init();
  }
  unlock();
}

void AUDIO_Flush(void) {
  lock();
  AUDIO.d.tick = 0;
  AUDIO.d.active = 0;
  AUDIO.sz.played = 0;
  AUDIO.sz.remaining = 0;
  unlock();
}

uint8_t AUDIO_Play(void) {
  uint8_t ok;
  audio_size_t *size = &(AUDIO.sz);

  lock();
  /* Get data size from audio file */
  size->remaining = SOUND_SIZE;

  /* Get total data to be played */
  if (size->remaining > AUDIO_BUFFER_SIZE)
    size->played = AUDIO_BUFFER_SIZE;
  else
    size->played = SOUND_SIZE;

  /* Start playing Wave */
  ok = AUDIO_OUT_Play((uint16_t *)SOUND_SAMPLE, size->played) == AUDIO_OK;
  unlock();

  return ok;
}

void AUDIO_BeepPlay(uint8_t Frequency, uint16_t TimeMS) {
  lock();

  cs43l22_SetBeep(Frequency, 0, 0);
  cs43l22_Beep(CS_BEEP_MODE_CONTINUOUS, CS_BEEP_MIX_ON);

  if (TimeMS > 0) {
    delayMs(TimeMS);
    cs43l22_Beep(CS_BEEP_MODE_OFF, CS_BEEP_MIX_ON);
  }

  unlock();
}

void AUDIO_BeepStop(void) {
  lock();

  cs43l22_Beep(CS_BEEP_MODE_OFF, CS_BEEP_MIX_ON);

  unlock();
}

uint8_t AUDIO_Mute(uint8_t mute) {
  return AUDIO_OUT_SetMute(mute ? CS_AUDIO_MUTE_ON : CS_AUDIO_MUTE_ON);
}

const audio_data_t* AUDIO_IO_Data(void) {
	return &(AUDIO.d);
}

/**
 * @brief   Pauses the audio file stream. In case of using DMA, the DMA Pause
 *          feature is used.
 * WARNING: When calling AUDIO_OUT_Pause() function for pause, only the
 *          AUDIO_OUT_Resume() function should be called for resume (use of
 * AUDIO_OUT_Play() function for resume could lead to unexpected behavior).
 * @retval  AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_Pause(void) {
  /* Call the Audio Codec Pause/Resume function */
  if (cs43l22_Pause() != 0)
    return AUDIO_ERROR;
  else {
    /* Call the Media layer pause function */
    HAL_I2S_DMAPause(AUDIO.pi2s);

    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief   Resumes the audio file streaming.
 * WARNING: When calling AUDIO_OUT_Pause() function for pause, only
 *          AUDIO_OUT_Resume() function should be called for resume (use of
 * AUDIO_OUT_Play() function for resume could lead to unexpected behavior).
 * @retval  AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_Resume(void) {
  /* Call the Audio Codec Pause/Resume function */
  if (cs43l22_Resume() != 0)
    return AUDIO_ERROR;
  else {
    /* Call the Media layer resume function */
    HAL_I2S_DMAResume(AUDIO.pi2s);

    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Stops audio playing and Power down the Audio Codec.
 * @param  Option: could be one of the following parameters
 *           - CS_CODEC_PDWN_HW: completely shut down the codec (physically).
 *                            Then need to reconfigure the Codec after power on.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_Stop(uint32_t Option) {
  /* Call DMA Stop to disable DMA stream before stopping codec */
  HAL_I2S_DMAStop(AUDIO.pi2s);

  /* Call Audio Codec Stop function */
  if (cs43l22_Stop(Option) != 0)
    return AUDIO_ERROR;
  else {
    if (Option == CS_CODEC_PDWN_HW) {
      /* Wait at least 1ms */
      delayMs(1);

      /* Reset the pin */
      GATE_AudioCodecStop();
    }

    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Controls the current audio volume level.
 * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for
 *         Mute and 100 for Max volume level).
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_SetVolume(uint8_t Volume) {
  if (AUDIO.d.volume == Volume) return AUDIO_OK;

  /* Call the codec volume control function with converted volume value */
  if (cs43l22_SetVolume(Volume) != 0)
    return AUDIO_ERROR;
  else {
    AUDIO.d.volume = Volume;
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Enables or disables the MUTE mode by software
 * @param  Cmd: could be CS_AUDIO_MUTE_ON to mute sound or CS_AUDIO_MUTE_OFF to
 *         unmute the codec and restore previous volume level.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_SetMute(uint32_t Cmd) {
  /* Call the Codec Mute function */
  if (cs43l22_SetMute(Cmd) != 0)
    return AUDIO_ERROR;
  else {
    AUDIO.d.mute = Cmd == CS_AUDIO_MUTE_ON;
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Switch dynamically (while audio file is played) the output target
 *         (speaker or headphone).
 * @note   This function modifies a global variable of the audio codec driver:
 * OutputDev.
 * @param  Output: specifies the audio output target: CS_OUT_DEV_SPEAKER,
 *         CS_OUT_DEV_HEADPHONE, CS_OUT_DEV_BOTH or CS_OUT_DEV_AUTO
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_SetOutputMode(uint8_t Output) {
  /* Call the Codec output Device function */
  if (cs43l22_SetOutputMode(Output) != 0)
    return AUDIO_ERROR;
  else
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
}

/**
 * @brief  Update the audio frequency.
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @note   This API should be called after the AUDIO_OUT_Init() to adjust the
 *         audio frequency.
 */
void AUDIO_OUT_SetFrequency(uint32_t AudioFreq) {
  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
  AUDIO_OUT_ClockConfig(AUDIO.pi2s, AudioFreq, NULL);

  /* Update the I2S audio frequency configuration */
  I2S_Init(AudioFreq);
}

/**
 * @brief  Clock Config.
 * @param  hi2s: might be required to set audio peripheral predivider if any.
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @note   This API is called by AUDIO_OUT_Init() and AUDIO_OUT_SetFrequency()
 *         Being __weak it can be overwritten by the application
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 */
__weak void AUDIO_OUT_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq,
                                  void *Params) {
  RCC_PeriphCLKInitTypeDef rccclkinit;
  uint8_t index = 0, freqindex = 0xFF;

  for (index = 0; index < 8; index++) {
    if (AUDIO.i2s.freq[index] == AudioFreq) {
      freqindex = index;
      break;
    }
  }

  /* Enable PLLI2S clock */
  HAL_RCCEx_GetPeriphCLKConfig(&rccclkinit);
  if ((freqindex & 0x7) == 0) {
    /* I2S clock config
PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) \D7 (PLLI2SN/PLLM)
I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    rccclkinit.PLLI2S.PLLI2SN = AUDIO.i2s.plln[freqindex];
    rccclkinit.PLLI2S.PLLI2SR = AUDIO.i2s.pllr[freqindex];
  } else {
    /* I2S clock config
PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) \D7 (PLLI2SN/PLLM)
I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    rccclkinit.PLLI2S.PLLI2SN = 258;
    rccclkinit.PLLI2S.PLLI2SR = 3;
  }
  rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S_APB1;
  HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
}

/**
 * @brief  AUDIO OUT I2S MSP Init.
 * @param  hi2s: might be required to set audio peripheral predivider if any.
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 */
void AUDIO_OUT_MspInit(I2S_HandleTypeDef *hi2s, void *Params) {
  HAL_I2S_MspInit(hi2s);
}

/**
 * @brief  De-Initializes AUDIO_OUT MSP.
 * @param  hi2s: might be required to set audio peripheral predivider if any.
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 */
void AUDIO_OUT_MspDeInit(I2S_HandleTypeDef *hi2s, void *Params) {
  HAL_I2S_MspDeInit(hi2s);
}

/**
 * @brief  Sends n-Bytes on the I2S interface.
 * @param  pData: Pointer to data address
 * @param  Size: Number of data to be written
 */
void AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size) {
  AUDIO.d.tick = tickMs();
  HAL_I2S_Transmit_DMA(AUDIO.pi2s, pData, DMA_MAX(Size / AUDIO_DATA_SZ));
}

/**
 * @brief  Manages the DMA Half Transfer complete event.
 */
__weak void AUDIO_OUT_HalfTransfer_CallBack(void) {
  audio_size_t *size = &(AUDIO.sz);

  // decrease remaining buffer
  size->remaining -= size->played;

  // done, repeat
  if (size->remaining == 0) size->remaining = SOUND_SIZE;

  // check remaining data
  if (size->remaining > AUDIO_BUFFER_SIZE)
    size->played = AUDIO_BUFFER_SIZE;
  else
    size->played = size->remaining;
}

/**
 * @brief  Manages the DMA full Transfer complete event.
 */
__weak void AUDIO_OUT_TransferComplete_CallBack(void) {
  audio_size_t *size = &(AUDIO.sz);
  uint32_t offset = (SOUND_SIZE - size->remaining) / AUDIO_DATA_SZ;

  // play it
  AUDIO_OUT_ChangeBuffer((uint16_t *)(SOUND_SAMPLE + offset), size->played);
}

/**
 * @brief  Manages the DMA FIFO error event.
 */
__weak void AUDIO_OUT_Error_CallBack(void) {}

/**
 * @brief  Tx Transfer completed callbacks.
 * @param  hi2s: I2S handle
 */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == AUDIO.pi2s->Instance)
    /* Call the user function which will manage directly transfer complete */
    AUDIO_OUT_TransferComplete_CallBack();
}

/**
 * @brief  Tx Half Transfer completed callbacks.
 * @param  hi2s: I2S handle
 */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == AUDIO.pi2s->Instance)
    /* Manage the remaining file size and new address offset: This function
should be coded by user (its prototype is already declared in
stm32f4_discovery_AUDIO.h) */
    AUDIO_OUT_HalfTransfer_CallBack();
}

/* Private functions implementation
 * --------------------------------------------*/
/**
 * @brief  Configures the audio peripherals.
 * @param  OutputDevice: CS_OUT_DEV_SPEAKER, CS_OUT_DEV_HEADPHONE,
 *                       CS_OUT_DEV_BOTH or CS_OUT_DEV_AUTO .
 * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
static uint8_t AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume,
                              uint32_t AudioFreq) {
  uint8_t ret = AUDIO_OK;

  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
  AUDIO_OUT_ClockConfig(AUDIO.pi2s, AudioFreq, NULL);

  /* I2S data transfer preparation:
Prepare the Media to be used for the audio transfer from memory to I2S
peripheral */
  if (HAL_I2S_GetState(AUDIO.pi2s) == HAL_I2S_STATE_RESET)
    /* Init the I2S MSP: this __weak function can be redefined by the
     * application*/
    AUDIO_OUT_MspInit(AUDIO.pi2s, NULL);

  /* I2S data transfer preparation:
Prepare the Media to be used for the audio transfer from memory to I2S
peripheral */
  /* Configure the I2S peripheral */
  if (I2S_Init(AudioFreq) != AUDIO_OK) ret = AUDIO_ERROR;

  if (ret == AUDIO_OK) {
    /* Retrieve audio codec identifier */
    if (AUDIO_Probe())
      cs43l22_Init(OutputDevice, Volume, AudioFreq);
    else
      ret = AUDIO_ERROR;
  }

  if (ret == AUDIO_OK) {
    if (AUDIO_Play())
      AUDIO.d.tick = tickMs();
    else
      ret = AUDIO_ERROR;
  }

  return ret;
}

static void AUDIO_OUT_DeInit(void) {
  AUDIO_OUT_Stop(CS_CODEC_PDWN_HW);
  cs43l22_DeInit();
  AUDIO_OUT_MspDeInit(AUDIO.pi2s, NULL);
}

/**
 * @brief  Starts playing audio stream from a data buffer for a determined size.
 * @param  Buffer: Pointer to the buffer
 * @param  Size: Number of audio data BYTES.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
static uint8_t AUDIO_OUT_Play(uint16_t *Buffer, uint32_t Size) {
  /* Call the audio Codec Play function */
  if (cs43l22_Play() != 0) return AUDIO_ERROR;

  /* Update the Media layer and enable it for play */
  if (HAL_I2S_Transmit_DMA(AUDIO.pi2s, Buffer, DMA_MAX(Size / AUDIO_DATA_SZ)) !=
      HAL_OK)
    return AUDIO_ERROR;
  else
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
}

/**
 * @brief  Initializes the Audio Codec audio interface (I2S).
 * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral.
 */
static uint8_t I2S_Init(uint32_t AudioFreq) {
  I2S_InitTypeDef *Init = &(AUDIO.pi2s->Init);

  /* Disable I2S block */
  __HAL_I2S_DISABLE(AUDIO.pi2s);

  Init->Mode = I2S_MODE_MASTER_TX;
  Init->Standard = I2S_STANDARD_PHILIPS;
  Init->DataFormat = I2S_DATAFORMAT_32B;
  Init->MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  Init->AudioFreq = AudioFreq;
  Init->CPOL = I2S_CPOL_LOW;
  Init->ClockSource = I2S_CLOCK_PLL;
  Init->FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

  /* Initialize the I2S peripheral with the structure above */
  if (HAL_I2S_Init(AUDIO.pi2s) != HAL_OK) return AUDIO_ERROR;

  return AUDIO_OK;
}

static void lock(void) {
#if (APP)
  osMutexAcquire(AudioRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(AudioRecMutexHandle);
#endif
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
