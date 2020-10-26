/**
 ******************************************************************************
 * @file    cs43l22.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the cs43l22.c driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#ifndef CS43L22_H_
#define CS43L22_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
// #define VERIFY_WRITTENDATA
/* Audio Reset Pin definition */
#define AUDIO_RESET_PIN                 INT_AUDIO_RST_Pin
#define AUDIO_RESET_GPIO                INT_AUDIO_RST_GPIO_Port

/* Maximum Timeout values for flags waiting loops. These timeouts are not based
 // on accurate values, they just guarantee that the application will not remain
 // stuck if the SPI communication is corrupted.
 // You may modify these timeout values depending on CPU frequency and application
 // conditions (interrupts routines ...). */
#define I2Cx_TIMEOUT_MAX                0x1000 /*<! The value of the maximal timeout for BUS waiting loops */

/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER         1
#define OUTPUT_DEVICE_HEADPHONE       2
#define OUTPUT_DEVICE_BOTH            3
#define OUTPUT_DEVICE_AUTO            4

/* Codec audio Standards */
#define CODEC_STANDARD                0x04

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

/* AUDIO FREQUENCY */
#define AUDIO_FREQUENCY_192K          ((uint32_t)192000)
#define AUDIO_FREQUENCY_96K           ((uint32_t)96000)
#define AUDIO_FREQUENCY_48K           ((uint32_t)48000)
#define AUDIO_FREQUENCY_44K           ((uint32_t)44100)
#define AUDIO_FREQUENCY_32K           ((uint32_t)32000)
#define AUDIO_FREQUENCY_22K           ((uint32_t)22050)
#define AUDIO_FREQUENCY_16K           ((uint32_t)16000)
#define AUDIO_FREQUENCY_11K           ((uint32_t)11025)
#define AUDIO_FREQUENCY_8K            ((uint32_t)8000)

/* BEEP Tone */
// Beep frequency
#define BEEP_FREQ_260_HZ			    0
#define BEEP_FREQ_521_HZ			    1
#define BEEP_FREQ_585_HZ			    2
#define BEEP_FREQ_666_HZ			    3
#define BEEP_FREQ_705_HZ			    4
#define BEEP_FREQ_774_HZ				5
#define BEEP_FREQ_888_HZ				6
#define BEEP_FREQ_1000_HZ				7
#define BEEP_FREQ_1043_HZ				8
#define BEEP_FREQ_1200_HZ				9
#define BEEP_FREQ_1333_HZ				10
#define BEEP_FREQ_1411_HZ				11
#define BEEP_FREQ_1600_HZ				12
#define BEEP_FREQ_1714_HZ				13
#define BEEP_FREQ_2000_HZ				14
#define BEEP_FREQ_2181_HZ				15
// Beep on time
#define BEEP_ON_86_MS					0
#define BEEP_ON_430_MS					1
#define BEEP_ON_780_MS					2
#define BEEP_ON_1200_MS					3
#define BEEP_ON_1500_MS					4
#define BEEP_ON_1800_MS					5
#define BEEP_ON_2200_MS					6
#define BEEP_ON_2500_MS					7
#define BEEP_ON_2800_MS					9
#define BEEP_ON_3200_MS					9
#define BEEP_ON_3500_MS					10
#define BEEP_ON_3800_MS					11
#define BEEP_ON_4200_MS					12
#define BEEP_ON_4500_MS					13
#define BEEP_ON_4800_MS					14
#define BEEP_ON_5200_MS					15
// Beep off time
#define BEEP_OFF_1230_MS				0
#define BEEP_OFF_2580_MS				1
#define BEEP_OFF_3900_MS				2
#define BEEP_OFF_5200_MS				3
#define BEEP_OFF_6600_MS				4
#define BEEP_OFF_8050_MS				5
#define BEEP_OFF_9350_MS				6
#define BEEP_OFF_10800_MS				7
// Beep volume
#define BEEP_VOL_MAX					6
#define BEEP_VOL_NORMAL					0
#define BEEP_VOL_MIN					7
// Beep mode
#define	BEEP_MODE_OFF					0
#define	BEEP_MODE_SINGLE				1
#define	BEEP_MODE_MULTIPLE				2
#define	BEEP_MODE_CONTINUOUS			3
// Mix with serial sound
#define BEEP_MIX_ON					    0
#define BEEP_MIX_OFF					1

/** CS43l22 Registers  ***/
#define   CS43L22_REG_ID                  0x01
#define   CS43L22_REG_POWER_CTL1          0x02
#define   CS43L22_REG_POWER_CTL2          0x04
#define   CS43L22_REG_CLOCKING_CTL        0x05
#define   CS43L22_REG_INTERFACE_CTL1      0x06
#define   CS43L22_REG_INTERFACE_CTL2      0x07
#define   CS43L22_REG_PASSTHR_A_SELECT    0x08
#define   CS43L22_REG_PASSTHR_B_SELECT    0x09
#define   CS43L22_REG_ANALOG_ZC_SR_SETT   0x0A
#define   CS43L22_REG_PASSTHR_GANG_CTL    0x0C
#define   CS43L22_REG_PLAYBACK_CTL1       0x0D
#define   CS43L22_REG_MISC_CTL            0x0E
#define   CS43L22_REG_PLAYBACK_CTL2       0x0F
#define   CS43L22_REG_PASSTHR_A_VOL       0x14
#define   CS43L22_REG_PASSTHR_B_VOL       0x15
#define   CS43L22_REG_PCMA_VOL            0x1A
#define   CS43L22_REG_PCMB_VOL            0x1B
#define   CS43L22_REG_BEEP_FREQ_ON_TIME   0x1C
#define   CS43L22_REG_BEEP_VOL_OFF_TIME   0x1D
#define   CS43L22_REG_BEEP_TONE_CFG       0x1E
#define   CS43L22_REG_TONE_CTL            0x1F
#define   CS43L22_REG_MASTER_A_VOL        0x20
#define   CS43L22_REG_MASTER_B_VOL        0x21
#define   CS43L22_REG_HEADPHONE_A_VOL     0x22
#define   CS43L22_REG_HEADPHONE_B_VOL     0x23
#define   CS43L22_REG_SPEAKER_A_VOL       0x24
#define   CS43L22_REG_SPEAKER_B_VOL       0x25
#define   CS43L22_REG_CH_MIXER_SWAP       0x26
#define   CS43L22_REG_LIMIT_CTL1          0x27
#define   CS43L22_REG_LIMIT_CTL2          0x28
#define   CS43L22_REG_LIMIT_ATTACK_RATE   0x29
#define   CS43L22_REG_OVF_CLK_STATUS      0x2E
#define   CS43L22_REG_BATT_COMPENSATION   0x2F
#define   CS43L22_REG_VP_BATTERY_LEVEL    0x30
#define   CS43L22_REG_SPEAKER_STATUS      0x31
#define   CS43L22_REG_TEMPMONITOR_CTL     0x32
#define   CS43L22_REG_THERMAL_FOLDBACK    0x33
#define   CS43L22_REG_CHARGE_PUMP_FREQ    0x34

/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
/**
 * @brief  CS43L22 ID
 */
#define  CS43L22_ID            0xE0
#define  CS43L22_ID_MASK       0xF8
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
#define CS43L22_CHIPID_ADDR    0x01

/* Public functions prototype ------------------------------------------------*/
uint32_t cs43l22_Init(uint16_t DeviceAddr, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
void cs43l22_DeInit(void);
uint32_t cs43l22_ReadID(uint16_t DeviceAddr);
uint32_t cs43l22_Play(uint16_t DeviceAddr, uint16_t *pBuffer, uint16_t Size);
uint32_t cs43l22_Pause(uint16_t DeviceAddr);
uint32_t cs43l22_Resume(uint16_t DeviceAddr);
uint32_t cs43l22_Stop(uint16_t DeviceAddr, uint32_t Cmd);
uint32_t cs43l22_SetVolume(uint16_t DeviceAddr, uint8_t Volume);
uint32_t cs43l22_SetFrequency(uint16_t DeviceAddr, uint32_t AudioFreq);
uint32_t cs43l22_SetMute(uint16_t DeviceAddr, uint32_t Cmd);
uint32_t cs43l22_SetOutputMode(uint16_t DeviceAddr, uint8_t Output);
uint32_t cs43l22_Reset(uint16_t DeviceAddr);
uint32_t cs43l22_SetBeep(uint16_t DeviceAddr, uint8_t Frequency, uint8_t OnTime, uint8_t OffTime);
uint32_t cs43l22_Beep(uint16_t DeviceAddr, uint8_t Mode, uint8_t Mix);

#endif /* CS43L22_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
