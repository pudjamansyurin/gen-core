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

/* Includes ------------------------------------------------------------------*/
#include "_audio.h"

/* Private constants ----------------------------------------------------------*/
#define AUDIO_BUFFER_SIZE             4096

/* External variabless ---------------------------------------------------------*/
extern osMutexId BeepMutexHandle;
extern AUDIO_DrvTypeDef cs43l22_drv;
extern I2S_HandleTypeDef hi2s3;
extern uint32_t AUDIO_SAMPLE_FREQ;
extern uint32_t AUDIO_SAMPLE_SIZE;
extern uint16_t AUDIO_SAMPLE[];

/* Private variables ---------------------------------------------------------*/
static AUDIO_DrvTypeDef *pAudioDrv;
static uint8_t AudioVolume = 0, AudioPlayDone = 0;
static uint16_t AudioPlaySize;
static uint32_t AudioRemSize;
/* These PLL parameters are valid when the f(VCO clock) = 1Mhz */
static const uint32_t I2SFreq[7] = { 8000, 16000, 22050, 32000, 44100, 48000, 96000 };
static const uint32_t I2SPLLN[7] = { 256, 213, 429, 213, 271, 258, 344 };
static const uint32_t I2SPLLR[7] = { 5, 2, 4, 2, 2, 3, 2 };

/* Private functions prototype ------------------------------------------------*/
static uint8_t I2S3_Init(uint32_t AudioFreq);
static uint8_t AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
static uint8_t AUDIO_OUT_Play(uint16_t *pBuffer, uint32_t Size);

/* Public functions implementation ---------------------------------------------*/
void AUDIO_Init(void) {
  uint8_t ret;
  do {
    LOG_StrLn("Wave_Init");

    // Mosftet control
    HAL_GPIO_WritePin(INT_AUDIO_PWR_GPIO_Port, INT_AUDIO_PWR_Pin, 0);
    osDelay(500);
    HAL_GPIO_WritePin(INT_AUDIO_PWR_GPIO_Port, INT_AUDIO_PWR_Pin, 1);
    osDelay(500);

    /* Initialize Wave player (Codec, DMA, I2C) */
    ret = AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, AudioVolume, AUDIO_SAMPLE_FREQ);

    osDelay(500);
  } while (ret != AUDIO_OK);
}

void AUDIO_Play(void) {
  /* Get data size from audio file */
  AudioRemSize = AUDIO_SAMPLE_SIZE;
  /* Get total data to be played */
  if (AUDIO_SAMPLE_SIZE > AUDIO_BUFFER_SIZE) {
    AudioPlaySize = AUDIO_BUFFER_SIZE;
  } else {
    AudioPlaySize = AUDIO_SAMPLE_SIZE;
  }

  /* Start playing Wave */
  AUDIO_OUT_Play((uint16_t*) AUDIO_SAMPLE, AudioPlaySize);
}

void AUDIO_BeepPlay(uint8_t Frequency, uint16_t TimeMS) {
  osRecursiveMutexWait(BeepMutexHandle, osWaitForever);
  pAudioDrv->SetBeep(AUDIO_I2C_ADDRESS, Frequency, 0, 0);
  pAudioDrv->Beep(AUDIO_I2C_ADDRESS, BEEP_MODE_CONTINUOUS, BEEP_MIX_ON);

  if (TimeMS > 0) {
    // delay with RTOS
    osDelay(TimeMS);
    // than stop
    pAudioDrv->Beep(AUDIO_I2C_ADDRESS, BEEP_MODE_OFF, BEEP_MIX_ON);
  }
  osRecursiveMutexRelease(BeepMutexHandle);
}

void AUDIO_BeepStop(void) {
  osRecursiveMutexWait(BeepMutexHandle, osWaitForever);

  pAudioDrv->Beep(AUDIO_I2C_ADDRESS, BEEP_MODE_OFF, BEEP_MIX_ON);

  osRecursiveMutexRelease(BeepMutexHandle);
}

/**
 * @brief  Sends n-Bytes on the I2S interface.
 * @param  pData: Pointer to data address
 * @param  Size: Number of data to be written
 */
void AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size) {
  HAL_I2S_Transmit_DMA(&hi2s3, pData, DMA_MAX(Size/AUDIODATA_SIZE));
}

/**
 * @brief   Pauses the audio file stream. In case of using DMA, the DMA Pause
 *          feature is used.
 * WARNING: When calling AUDIO_OUT_Pause() function for pause, only the
 *          AUDIO_OUT_Resume() function should be called for resume (use of AUDIO_OUT_Play()
 *          function for resume could lead to unexpected behavior).
 * @retval  AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_Pause(void) {
  /* Call the Audio Codec Pause/Resume function */
  if (pAudioDrv->Pause(AUDIO_I2C_ADDRESS) != 0) {
    return AUDIO_ERROR;
  } else {
    /* Call the Media layer pause function */
    HAL_I2S_DMAPause(&hi2s3);

    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief   Resumes the audio file streaming.
 * WARNING: When calling AUDIO_OUT_Pause() function for pause, only
 *          AUDIO_OUT_Resume() function should be called for resume (use of AUDIO_OUT_Play()
 *          function for resume could lead to unexpected behavior).
 * @retval  AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_Resume(void) {
  /* Call the Audio Codec Pause/Resume function */
  if (pAudioDrv->Resume(AUDIO_I2C_ADDRESS) != 0) {
    return AUDIO_ERROR;
  } else {
    /* Call the Media layer resume function */
    HAL_I2S_DMAResume(&hi2s3);

    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Stops audio playing and Power down the Audio Codec.
 * @param  Option: could be one of the following parameters
 *           - CODEC_PDWN_HW: completely shut down the codec (physically).
 *                            Then need to reconfigure the Codec after power on.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_Stop(uint32_t Option) {
  /* Call DMA Stop to disable DMA stream before stopping codec */
  HAL_I2S_DMAStop(&hi2s3);

  /* Call Audio Codec Stop function */
  if (pAudioDrv->Stop(AUDIO_I2C_ADDRESS, Option) != 0) {
    return AUDIO_ERROR;
  } else {
    if (Option == CODEC_PDWN_HW) {
      /* Wait at least 1ms */
      osDelay(1);

      /* Reset the pin */
      HAL_GPIO_WritePin(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, GPIO_PIN_RESET);
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
  /* Call the codec volume control function with converted volume value */
  if (pAudioDrv->SetVolume(AUDIO_I2C_ADDRESS, Volume) != 0) {
    return AUDIO_ERROR;
  } else {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Enables or disables the MUTE mode by software
 * @param  Cmd: could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to
 *         unmute the codec and restore previous volume level.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_SetMute(uint32_t Cmd) {
  /* Call the Codec Mute function */
  if (pAudioDrv->SetMute(AUDIO_I2C_ADDRESS, Cmd) != 0) {
    return AUDIO_ERROR;
  } else {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Switch dynamically (while audio file is played) the output target
 *         (speaker or headphone).
 * @note   This function modifies a global variable of the audio codec driver: OutputDev.
 * @param  Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
 *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
uint8_t AUDIO_OUT_SetOutputMode(uint8_t Output) {
  /* Call the Codec output Device function */
  if (pAudioDrv->SetOutputMode(AUDIO_I2C_ADDRESS, Output) != 0) {
    return AUDIO_ERROR;
  } else {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Update the audio frequency.
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @note   This API should be called after the AUDIO_OUT_Init() to adjust the
 *         audio frequency.
 */
void AUDIO_OUT_SetFrequency(uint32_t AudioFreq) {
  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
  AUDIO_OUT_ClockConfig(&hi2s3, AudioFreq, NULL);

  /* Update the I2S audio frequency configuration */
  I2S3_Init(AudioFreq);
}

/**
 * @brief  Clock Config.
 * @param  hi2s: might be required to set audio peripheral predivider if any.
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @note   This API is called by AUDIO_OUT_Init() and AUDIO_OUT_SetFrequency()
 *         Being __weak it can be overwritten by the application
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 */
__weak void AUDIO_OUT_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq, void *Params) {
  RCC_PeriphCLKInitTypeDef rccclkinit;
  uint8_t index = 0, freqindex = 0xFF;

  for (index = 0; index < 7; index++) {
    if (I2SFreq[index] == AudioFreq) {
      freqindex = index;
      break;
    }
  }

  /* Enable PLLI2S clock */
  HAL_RCCEx_GetPeriphCLKConfig(&rccclkinit);
  /* PLLI2S_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  if (freqindex != 0xFF) {
    /* I2S clock config
     PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) � (PLLI2SN/PLLM)
     I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    rccclkinit.PLLI2S.PLLI2SN = I2SPLLN[freqindex];
    rccclkinit.PLLI2S.PLLI2SR = I2SPLLR[freqindex];
  } else {
    /* I2S clock config
     PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) � (PLLI2SN/PLLM)
     I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
    rccclkinit.PLLI2S.PLLI2SN = 258;
    rccclkinit.PLLI2S.PLLI2SR = 3;
  }
  rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;

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
 * @brief  Manages the DMA full Transfer complete event.
 */
__weak void AUDIO_OUT_TransferComplete_CallBack(void) {
  if (!AudioPlayDone) {
    AUDIO_OUT_ChangeBuffer((uint16_t*) (AUDIO_SAMPLE + ((AUDIO_SAMPLE_SIZE - AudioRemSize) / AUDIODATA_SIZE)), AudioPlaySize);
  } else {
    /* Get data size from audio file */
    AudioRemSize = AUDIO_SAMPLE_SIZE;
    /* Get total data to be played */
    if (AUDIO_SAMPLE_SIZE > AUDIO_BUFFER_SIZE) {
      AudioPlaySize = AUDIO_BUFFER_SIZE;
    } else {
      AudioPlaySize = AUDIO_SAMPLE_SIZE;
    }

    /* Start playing Wave again*/
    AUDIO_OUT_ChangeBuffer((uint16_t*) AUDIO_SAMPLE, AudioPlaySize);
  }

  AudioPlayDone = (AudioPlaySize == AudioRemSize);
}

/**
 * @brief  Manages the DMA Half Transfer complete event.
 */
__weak void AUDIO_OUT_HalfTransfer_CallBack(void) {
  // check remaining data
  if (AudioRemSize > AUDIO_BUFFER_SIZE) {
    /* Get total data to be played */
    AudioPlaySize = AUDIO_BUFFER_SIZE;
    /* Get remaining data */
    AudioRemSize -= AUDIO_BUFFER_SIZE;
  } else {
    /* Get total data to be played */
    if (!AudioPlayDone) {
      AudioPlaySize = AudioRemSize;
    }
  }
}

/**
 * @brief  Manages the DMA FIFO error event.
 */
__weak void AUDIO_OUT_Error_CallBack(void) {
}

/**
 * @brief  Tx Transfer completed callbacks.
 * @param  hi2s: I2S handle
 */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == I2S) {
    /* Call the user function which will manage directly transfer complete */
    AUDIO_OUT_TransferComplete_CallBack();
  }
}

/**
 * @brief  Tx Half Transfer completed callbacks.
 * @param  hi2s: I2S handle
 */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == I2S) {
    /* Manage the remaining file size and new address offset: This function should
     be coded by user (its prototype is already declared in stm32f4_discovery_audio.h) */
    AUDIO_OUT_HalfTransfer_CallBack();
  }
}

/* Private functions implementation ---------------------------------------------*/
/**
 * @brief  Configures the audio peripherals.
 * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
 *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
 * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
static uint8_t AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq) {
  uint8_t ret = AUDIO_OK;

  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
  AUDIO_OUT_ClockConfig(&hi2s3, AudioFreq, NULL);

  /* I2S data transfer preparation:
   Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
  if (HAL_I2S_GetState(&hi2s3) == HAL_I2S_STATE_RESET) {
    /* Init the I2S MSP: this __weak function can be redefined by the application*/
    AUDIO_OUT_MspInit(&hi2s3, NULL);
  }

  /* I2S data transfer preparation:
   Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
  /* Configure the I2S peripheral */
  if (I2S3_Init(AudioFreq) != AUDIO_OK) {
    ret = AUDIO_ERROR;
  }

  if (ret == AUDIO_OK) {
    /* Retieve audio codec identifier */
    if (((cs43l22_drv.ReadID(AUDIO_I2C_ADDRESS)) & CS43L22_ID_MASK) == CS43L22_ID) {
      /* Initialize the audio driver structure */
      pAudioDrv = &cs43l22_drv;
    } else {
      ret = AUDIO_ERROR;
    }

  }

  if (ret == AUDIO_OK) {
    pAudioDrv->Init(AUDIO_I2C_ADDRESS, OutputDevice, Volume, AudioFreq);
  }

  return ret;
}

/**
 * @brief  Starts playing audio stream from a data buffer for a determined size.
 * @param  pBuffer: Pointer to the buffer
 * @param  Size: Number of audio data BYTES.
 * @retval AUDIO_OK if correct communication, else wrong communication
 */
static uint8_t AUDIO_OUT_Play(uint16_t *pBuffer, uint32_t Size) {
  /* Call the audio Codec Play function */
  if (pAudioDrv->Play(AUDIO_I2C_ADDRESS, pBuffer, Size) != 0) {
    return AUDIO_ERROR;
  } else {
    /* Update the Media layer and enable it for play */

    HAL_I2S_Transmit_DMA(&hi2s3, pBuffer, DMA_MAX(Size/AUDIODATA_SIZE));
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
 * @brief  Initializes the Audio Codec audio interface (I2S).
 * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral.
 */
static uint8_t I2S3_Init(uint32_t AudioFreq) {
  /* Disable I2S block */
  __HAL_I2S_DISABLE(&hi2s3);

  /* I2S3 peripheral configuration */
  hi2s3.Init.AudioFreq = AudioFreq;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  // FIXME: Why it works on I2S_DATAFORMAT_32B? It should be I2S_DATAFORMAT_16B
  //	hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  /* Initialize the I2S peripheral with the structure above */
  if (HAL_I2S_Init(&hi2s3) != HAL_OK) {
    return AUDIO_ERROR;
  } else {
    return AUDIO_OK;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
