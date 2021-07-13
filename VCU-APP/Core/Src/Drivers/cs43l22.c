/**
 ******************************************************************************
 * @file    CS43.c
 * @author  MCD Application Team
 * @brief   This file provides the CS43L22 Audio Codec driver.
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

/* Includes
 * --------------------------------------------*/
#include "Drivers/cs43l22.h"

#include <math.h>

#include "Drivers/codec.h"


/* Private constants
 * --------------------------------------------*/
#define CS_DEV_ADDR 0x94

/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
/**
 * @brief  CS43L22 ID
 */
#define CS_ID 0xE0
#define CS_ID_MASK 0xF8
/**
 * @brief Chip ID Register: Chip I.D. and Revision Register
 *  Read only register
 *  Default value: 0x01
 *  [7:3] CHIPID[4:0]: I.D. code for the CS43L22.
 *        Default value: 11100b
 *  [2:0] REVID[2:0]: CS43L22 revision level.
 *        Default value:
 *        000 - Rev A0
 *        001 - Rev A1
 *        010 - Rev B0
 *        011 - Rev B1
 */
#define CS_CHIPID_ADDR 0x01

/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Codec audio Standards */
#define CS_CODEC_STANDARD 0x04

/* Vol Levels values */
//#define CS_DEF_VOL_MIN 0x00
//#define CS_DEF_VOL_MAX 0xFF
//#define CS_DEF_VOL_STEP 0x04

/* AUDIO FREQUENCY */
//#define CS_AUDIO_FREQ_192K (192000)
//#define CS_AUDIO_FREQ_96K (96000)
//#define CS_AUDIO_FREQ_48K (48000)
//#define CS_AUDIO_FREQ_44K (44100)
//#define CS_AUDIO_FREQ_32K (32000)
//#define CS_AUDIO_FREQ_22K (22050)
//#define CS_AUDIO_FREQ_16K (16000)
//#define CS_AUDIO_FREQ_11K (11025)
//#define CS_AUDIO_FREQ_8K (8000)

// Beep volume
#define CS_BEEP_VOL_NORMAL 0
#define CS_BEEP_VOL_MAX 6
#define CS_BEEP_VOL_MIN 7

/** CS43l22 Registers  ***/
#define CS_REG_ID 0x01
#define CS_REG_POWER_CTL1 0x02
#define CS_REG_POWER_CTL2 0x04
#define CS_REG_CLOCKING_CTL 0x05
#define CS_REG_INTERFACE_CTL1 0x06
#define CS_REG_INTERFACE_CTL2 0x07
#define CS_REG_PASSTHR_A_SELECT 0x08
#define CS_REG_PASSTHR_B_SELECT 0x09
#define CS_REG_ANALOG_ZC_SR_SETT 0x0A
#define CS_REG_PASSTHR_GANG_CTL 0x0C
#define CS_REG_PLAYBACK_CTL1 0x0D
#define CS_REG_MISC_CTL 0x0E
#define CS_REG_PLAYBACK_CTL2 0x0F
#define CS_REG_PASSTHR_A_VOL 0x14
#define CS_REG_PASSTHR_B_VOL 0x15
#define CS_REG_PCMA_VOL 0x1A
#define CS_REG_PCMB_VOL 0x1B
#define CS_REG_BEEP_FREQ_ON_TIME 0x1C
#define CS_REG_BEEP_VOL_OFF_TIME 0x1D
#define CS_REG_BEEP_TONE_CFG 0x1E
#define CS_REG_TONE_CTL 0x1F
#define CS_REG_MASTER_A_VOL 0x20
#define CS_REG_MASTER_B_VOL 0x21
#define CS_REG_HEADPHONE_A_VOL 0x22
#define CS_REG_HEADPHONE_B_VOL 0x23
#define CS_REG_SPEAKER_A_VOL 0x24
#define CS_REG_SPEAKER_B_VOL 0x25
#define CS_REG_CH_MIXER_SWAP 0x26
#define CS_REG_LIMIT_CTL1 0x27
#define CS_REG_LIMIT_CTL2 0x28
#define CS_REG_LIMIT_ATTACK_RATE 0x29
#define CS_REG_OVF_CLK_STATUS 0x2E
#define CS_REG_BATT_COMPENSATION 0x2F
#define CS_REG_VP_BATTERY_LEVEL 0x30
#define CS_REG_SPEAKER_STATUS 0x31
#define CS_REG_TEMPMONITOR_CTL 0x32
#define CS_REG_THERMAL_FOLDBACK 0x33
#define CS_REG_CHARGE_PUMP_FREQ 0x34

/* Private enums
 * --------------------------------------------*/
typedef enum {
  CS_AUDIO_PAUSE,
  CS_AUDIO_RESUME,
} CS_AUDIO;

// Beep on time
typedef enum {
  CS_BEEP_ON_86_MS,
  CS_BEEP_ON_430_MS,
  CS_BEEP_ON_780_MS,
  CS_BEEP_ON_1200_MS,
  CS_BEEP_ON_1500_MS,
  CS_BEEP_ON_1800_MS,
  CS_BEEP_ON_2200_MS,
  CS_BEEP_ON_2500_MS,
  CS_BEEP_ON_2800_MS,
  CS_BEEP_ON_3200_MS,
  CS_BEEP_ON_3500_MS,
  CS_BEEP_ON_3800_MS,
  CS_BEEP_ON_4200_MS,
  CS_BEEP_ON_4500_MS,
  CS_BEEP_ON_4800_MS,
  CS_BEEP_ON_5200_MS,
} CS_BEEP_ON;

// Beep off time
typedef enum {
  CS_BEEP_OFF_1230_MS,
  CS_BEEP_OFF_2580_MS,
  CS_BEEP_OFF_3900_MS,
  CS_BEEP_OFF_5200_MS,
  CS_BEEP_OFF_6600_MS,
  CS_BEEP_OFF_8050_MS,
  CS_BEEP_OFF_9350_MS,
  CS_BEEP_OFF_10800_MS,
} CS_BEEP_OFF;

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint8_t volume;
  uint8_t stopDevice;
  uint8_t outDev;
  uint16_t outDevice;
} cs43l22_t;

/* Private variables
 * --------------------------------------------*/
static cs43l22_t CS43 = {
    .stopDevice = 1,
    .outDev = 0,
};

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t ConvertVolume(uint8_t Vol);

/* Public functions implementation
 * --------------------------------------------*/
/**
 * @brief Initializes the audio codec and the control interface.
 * @param DevAddr: Device address on communication Bus.
 * @param OutputDevice: can be CS_OUT_DEV_SPEAKER, CS_OUT_DEV_HEADPHONE,
 *                       CS_OUT_DEV_BOTH or CS_OUT_DEV_AUTO .
 * @param Vol: Initial volume level (from 0 (Mute) to 100 (Max))
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_Init(uint16_t OutputDevice, uint8_t Vol, uint32_t AudioFreq) {
  uint32_t counter = 0;

  /* Initialize the Control interface of the Audio Codec */
  CODEC_Init(CS_DEV_ADDR);

  /* Keep Codec powered OFF */
  counter += CODEC_Write(CS_REG_POWER_CTL1, 0x01);

  /*Save Output device for mute ON/OFF procedure*/
  switch (OutputDevice) {
    case CS_OUT_DEV_SPEAKER:
      CS43.outDev = 0xFA;
      break;
    case CS_OUT_DEV_HEADPHONE:
      CS43.outDev = 0xAF;
      break;
    case CS_OUT_DEV_BOTH:
      CS43.outDev = 0xAA;
      break;
    case CS_OUT_DEV_AUTO:
      CS43.outDev = 0x05;
      break;
    default:
      CS43.outDev = 0x05;
      break;
  }

  counter += CODEC_Write(CS_REG_POWER_CTL2, CS43.outDev);

  /* Clock configuration: Auto detection */
  counter += CODEC_Write(CS_REG_CLOCKING_CTL, 0x80);

  /* Set the Slave Mode and the audio Standard */
  counter += CODEC_Write(CS_REG_INTERFACE_CTL1, CS_CODEC_STANDARD);

  /* Set the Master volume to maximum */
  counter += CODEC_Write(CS_REG_MASTER_A_VOL, 0);
  counter += CODEC_Write(CS_REG_MASTER_B_VOL, 0);

  /* If the Speaker is enabled, set the Mono mode and volume attenuation level
   */
  if (OutputDevice != CS_OUT_DEV_HEADPHONE) {
    /* Set the Speaker Mono mode */
    counter += CODEC_Write(CS_REG_PLAYBACK_CTL2, 0x06);
  }

  /* Set the Speaker/Headphone attenuation level */
  counter += CS43_SetVolume(Vol);

  /* Additional configuration for the CODEC. These configurations are done to
reduce the time needed for the Codec to power off. If these configurations are
removed, then a long delay should be added between powering off the Codec and
switching off the I2S peripheral MCLK clock (which is the operating clock for
Codec). If this delay is not inserted, then the codec will not shut down
properly and it results in high noise after shut down. */

  /* Disable the analog soft ramp */
  counter += CODEC_Write(CS_REG_ANALOG_ZC_SR_SETT, 0x00);
  /* Disable the digital soft ramp */
  counter += CODEC_Write(CS_REG_MISC_CTL, 0x04);
  /* Disable the limiter attack level */
  counter += CODEC_Write(CS_REG_LIMIT_CTL1, 0x00);
  /* Adjust Bass and Treble levels */
  counter += CODEC_Write(CS_REG_TONE_CTL, 0x0F);
  /* Adjust PCM volume level */
  counter += CODEC_Write(CS_REG_PCMA_VOL, 0x0A);
  counter += CODEC_Write(CS_REG_PCMB_VOL, 0x0A);

  /* Return communication control value */
  CS43.outDevice = OutputDevice;
  return counter;
}

/**
 * @brief  Deinitializes the audio codec.
 * @param  None
 * @retval  None
 */
void CS43_DeInit(void) {
  /* Deinitialize Audio Codec interface */
  CODEC_DeInit();
}

uint8_t CS43_Probe(void) { return (CS43_ReadID() & CS_ID_MASK) == CS_ID; }

/**
 * @brief  Get the CS43L22 ID.
 * @param DevAddr: Device address on communication Bus.
 * @retval The CS43L22 ID
 */
uint32_t CS43_ReadID(void) {
  uint8_t ok, Value = 0;

  /* Initialize the Control interface of the Audio Codec */
  ok = CODEC_Init(CS_DEV_ADDR);

  if (ok) {
    Value = CODEC_Read(CS_CHIPID_ADDR);
    Value = (Value & CS_ID_MASK);
  }

  return ((uint32_t)Value);
}

/**
 * @brief Start the audio Codec play feature.
 * @note For this codec no Play options are required.
 * @param DevAddr: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_Play(void) {
  uint32_t counter = 0;

  if (CS43.stopDevice == 1) {
    /* Enable the digital soft ramp */
    counter += CODEC_Write(CS_REG_MISC_CTL, 0x06);

    /* Enable Output device */
    counter += CS43_SetMute(CS_AUDIO_MUTE_OFF);

    /* Power on the Codec */
    counter += CODEC_Write(CS_REG_POWER_CTL1, 0x9E);
    CS43.stopDevice = 0;
  }

  /* Return communication control value */
  return counter;
}

/**
 * @brief Pauses playing on the audio codec.
 * @param DevAddr: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_Pause(void) {
  uint32_t counter = 0;

  /* Pause the audio file playing */
  /* Mute the output first */
  counter += CS43_SetMute(CS_AUDIO_MUTE_ON);

  /* Put the Codec in Power save mode */
  counter += CODEC_Write(CS_REG_POWER_CTL1, 0x01);

  return counter;
}

/**
 * @brief Resumes playing on the audio codec.
 * @param DevAddr: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_Resume(void) {
  uint32_t counter = 0;

  /* Resumes the audio file playing */
  /* Unmute the output first */
  counter += CS43_SetMute(CS_AUDIO_MUTE_OFF);

  for (uint8_t index = 0x00; index < 0xFF; index++)
    ;

  counter += CODEC_Write(CS_REG_POWER_CTL2, CS43.outDev);

  /* Exit the Power save mode */
  counter += CODEC_Write(CS_REG_POWER_CTL1, 0x9E);

  return counter;
}

/**
 * @brief Stops audio Codec playing. It powers down the codec.
 * @param DevAddr: Device address on communication Bus.
 * @param CodecPdwnMode: selects the  power down mode.
 *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from
 * this mode, the codec is set to default configuration (user should
 * re-Initialize the codec in order to play again the audio stream).
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_Stop(uint32_t CodecPdwnMode) {
  uint32_t counter = 0;

  /* Mute the output first */
  counter += CS43_SetMute(CS_AUDIO_MUTE_ON);

  /* Disable the digital soft ramp */
  counter += CODEC_Write(CS_REG_MISC_CTL, 0x04);

  /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
  counter += CODEC_Write(CS_REG_POWER_CTL1, 0x01);

  CS43.stopDevice = 1;
  return counter;
}

/**
 * @brief Sets higher or lower the codec volume level.
 * @param DevAddr: Device address on communication Bus.
 * @param Vol: a byte value from 0 to 255 (refer to codec registers
 *                description for more details).
 *
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_SetVolume(uint8_t Vol) {
  uint32_t counter = 0;
  uint16_t regA, regB;

  if (CS43.outDevice != CS_OUT_DEV_HEADPHONE) {
    regA = CS_REG_SPEAKER_A_VOL;
    regB = CS_REG_SPEAKER_B_VOL;
  } else {
    regA = CS_REG_HEADPHONE_A_VOL;
    regB = CS_REG_HEADPHONE_B_VOL;
  }

  counter += CODEC_Write(regA, ConvertVolume(Vol));
  counter += CODEC_Write(regB, ConvertVolume(Vol));

  CS43.volume = Vol;
  return counter;
}

/**
 * @brief Sets new frequency.
 * @param DevAddr: Device address on communication Bus.
 * @param AudioFreq: Audio frequency used to play the audio stream.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_SetFrequency(uint32_t AudioFreq) { return 0; }

/**
 * @brief Enables or disables the mute feature on the audio codec.
 * @param DevAddr: Device address on communication Bus.
 * @param Cmd: CS_AUDIO_MUTE_ON to enable the mute or CS_AUDIO_MUTE_OFF to
 * disable the mute mode.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_SetMute(uint32_t Cmd) {
  uint32_t counter = 0;
  uint16_t regA, regB;

  if (CS43.outDevice != CS_OUT_DEV_HEADPHONE) {
    regA = CS_REG_SPEAKER_A_VOL;
    regB = CS_REG_SPEAKER_B_VOL;
  } else {
    regA = CS_REG_HEADPHONE_A_VOL;
    regB = CS_REG_HEADPHONE_B_VOL;
  }

  /* Set the Mute mode */
  if (Cmd == CS_AUDIO_MUTE_ON) {
    counter += CODEC_Write(CS_REG_POWER_CTL2, 0xFF);
    // mute
    counter += CODEC_Write(regA, 0x01);
    counter += CODEC_Write(regB, 0x01);
  }
  /* CS_AUDIO_MUTE_OFF Disable the Mute */
  else {
    // set to max
    counter += CODEC_Write(regA, ConvertVolume(CS43.volume));
    counter += CODEC_Write(regB, ConvertVolume(CS43.volume));
    counter += CODEC_Write(CS_REG_POWER_CTL2, CS43.outDev);
  }
  return counter;
}

/**
 * @brief Switch dynamically (while audio file is played) the output target
 *         (speaker or headphone).
 * @note This function modifies a global variable of the audio codec driver:
 * OutputDev.
 * @param DevAddr: Device address on communication Bus.
 * @param Output: specifies the audio output target: CS_OUT_DEV_SPEAKER,
 *         CS_OUT_DEV_HEADPHONE, CS_OUT_DEV_BOTH or CS_OUT_DEV_AUTO
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_SetOutputMode(uint8_t Output) {
  uint32_t counter = 0;

  switch (Output) {
    case CS_OUT_DEV_SPEAKER:
      counter += CODEC_Write(CS_REG_POWER_CTL2,
                             0xFA); /* SPK always ON & HP always OFF */
      CS43.outDev = 0xFA;
      break;

    case CS_OUT_DEV_HEADPHONE:
      counter += CODEC_Write(CS_REG_POWER_CTL2,
                             0xAF); /* SPK always OFF & HP always ON */
      CS43.outDev = 0xAF;
      break;

    case CS_OUT_DEV_BOTH:
      counter += CODEC_Write(CS_REG_POWER_CTL2,
                             0xAA); /* SPK always ON & HP always ON */
      CS43.outDev = 0xAA;
      break;

    case CS_OUT_DEV_AUTO:
      counter += CODEC_Write(CS_REG_POWER_CTL2,
                             0x05); /* Detect the HP or the SPK automatically */
      CS43.outDev = 0x05;
      break;

    default:
      counter += CODEC_Write(CS_REG_POWER_CTL2,
                             0x05); /* Detect the HP or the SPK automatically */
      CS43.outDev = 0x05;
      break;
  }
  return counter;
}

/**
 * @brief Resets cs43l22 registers.
 * @param DevAddr: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t CS43_Reset(void) { return 0; }

uint32_t CS43_SetBeep(uint8_t Frequency, uint8_t OnTime, uint8_t OffTime) {
  uint32_t counter = 0;

  /* Set frequency of beep and on time */
  counter += CODEC_Write(CS_REG_BEEP_FREQ_ON_TIME, (Frequency << 4) | OnTime);
  /* Set volume of beep (max), and off time */
  counter +=
      CODEC_Write(CS_REG_BEEP_VOL_OFF_TIME, (OffTime << 5) | CS_BEEP_VOL_MAX);

  return counter;
}

uint32_t CS43_Beep(uint8_t Mode, uint8_t Mix) {
  uint32_t counter = 0;

  /* Set mode beep play and mix with serial sound*/
  counter += CODEC_Write(CS_REG_BEEP_TONE_CFG, (Mode << 6) | (Mix << 5));

  return counter;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t ConvertVolume(uint8_t Vol) {
  uint64_t V, Multiplier = pow(10, 10);
  uint8_t Log, Result;

  // change zero to 1
  Vol = Vol ? Vol : 1;

  // expand resolution
  V = Vol > 100 ? Multiplier : (uint64_t)((Vol * Multiplier) / 100);

  // convert linear to logarithmic (scale 100)
  Log = (uint8_t)(10 * log10(1 + V));

  // scale 100 to 255
  Result = Log > 100 ? 255 : (uint8_t)((Log * 255) / 100);

  return Result;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
