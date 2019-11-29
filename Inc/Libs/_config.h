/*
 * _config.h
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "main.h"
#include <string.h>									// for: strlen()
#include <stdlib.h>									// for: itoa()
#include <stdio.h>									// for: sprintf()
#include "_flash.h"
#include "_swv.h"

// definition
#define SIMCOM_EXTRA_TIME_MS				2000
#define GMT_TIME										7								// Asia/Jakarta
#define	REPORT_INTERVAL							5								// in second

// macro to manipulate bit
#define SetBit(x) 									(1 << x)
#define SetBitOf(var, x) 						(var |= 1 << x)
#define ClearBitOf(var, x) 					(var &= ~(1 << x))
#define ToggleBitOf(var, x) 				(var ^= 1 << x)

// list event
#define EVENT_IOT_SEND_REPORT 			SetBit(0)

#define EVENT_REPORTER_CRASH				SetBit(0)
#define EVENT_REPORTER_FALL					SetBit(1)
#define EVENT_REPORTER_FALL_FIXED		SetBit(2)

#define EVENT_AUDIO_BEEP						SetBit(0)
#define EVENT_AUDIO_MUTE_ON					SetBit(1)
#define EVENT_AUDIO_MUTE_OFF				SetBit(2)

#define EVENT_COMMAND_ARRIVED				SetBit(0)

#define EVENT_FINGER_PLACED					SetBit(0)

#define EVENT_KEYLESS_BROADCAST			SetBit(0)
#define EVENT_KEYLESS_FINDER				SetBit(1)
#define EVENT_KEYLESS_SEAT 					SetBit(2)

#define EVENT_CAN_RX_IT							SetBit(0)

#define EVENT_KEYLESS_RX_IT					SetBit(0)

// Function prototype
void BSP_Led_Write_All(uint8_t state);
void BSP_Led_Toggle_All(void);
void ftoa(float f, char *str, char size);
void BSP_Led_Disco(uint16_t ms);

#endif /* CONFIG_H_ */
