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

// macro to manipulate bit
#define SetBit(x) 									(1 << x)
#define SetBitOf(var, x) 						(var |= 1 << x)
#define ClearBitOf(var, x) 					(var &= ~(1 << x))
#define ToggleBitOf(var, x) 				(var ^= 1 << x)
#define CHARISNUM(x)                ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                ((x) - '0')
// Function prototype
void BSP_LedWrite(uint8_t state);
void BSP_LedToggle(void);
void BSP_LedDisco(uint16_t ms);
int8_t BSP_BitPosition(uint64_t event_id);
uint32_t BSP_ChangeEndian32(uint32_t val);
uint64_t BSP_ChangeEndian64(uint64_t val);
int32_t BSP_ParseNumber(const char *ptr, uint8_t *cnt);
float BSP_ParseFloatNumber(const char *ptr, uint8_t *cnt);
// FIXME: remove me if unused
//void ftoa(float f, char *str, char size);

// GLOBAL CONFIG
#define VCU_FIRMWARE_VERSION				"0.7"
#define VCU_VENDOR									"GEN Indonesia"
#define VCU_BUILD_YEAR							20U

#define NET_SERVER_IP								"36.80.66.101"
#define NET_SERVER_PORT							5044
#define NET_APN											"3gprs"					// "telkomsel"
#define	NET_APN_USERNAME						"3gprs"					// "wap"
#define NET_APN_PASSWORD						"3gprs"					// "wap123"
#define NET_SIGNAL									2								// 2=AUTO, 13=2G, 14=3G
#define NET_BOOT_TIMEOUT						10							// in second
#define NET_REPEAT_DELAY						5								// in second
#define NET_EXTRA_TIME_MS						500 						// in ms

#define NET_COMMAND_PREFIX					"$T"
#define NET_ACK_PREFIX							"@C"

#define FINGER_CONFIDENCE_MIN 			10
#define FINGER_SCAN_TIMEOUT					20							// in second

#define	REPORT_INTERVAL_SIMPLE			5								// in second
#define	REPORT_INTERVAL_FULL				20							// in second
#define REPORT_UNITID								354313U
#define FRAME_PREFIX								0x4047 					// "@G"

#define GMT_TIME										7								// Asia/Jakarta

// Event List (RTOS Tasks)
#define EVENT_IOT_RESPONSE 					SetBit(0)
#define EVENT_IOT_REPORT						SetBit(1)

#define EVENT_REPORTER_CRASH				SetBit(0)
#define EVENT_REPORTER_FALL					SetBit(1)
#define EVENT_REPORTER_FALL_FIXED		SetBit(2)

#define EVENT_AUDIO_BEEP						SetBit(0)
#define EVENT_AUDIO_MUTE_ON					SetBit(1)
#define EVENT_AUDIO_MUTE_OFF				SetBit(2)

#define EVENT_FINGER_PLACED					SetBit(0)

#define EVENT_CAN_RX_IT							SetBit(0)

#define EVENT_KEYLESS_RX_IT					SetBit(0)

// Payload list (Keyless)
#define KEYLESS_MSG_BROADCAST				SetBit(0)
#define KEYLESS_MSG_FINDER					SetBit(1)
#define KEYLESS_MSG_SEAT 						SetBit(2)

// Events group (Frame Report)
#define REPORT_NETWORK_RESTART			SetBit(0)
#define REPORT_BIKE_FALLING 				SetBit(1)
#define REPORT_BIKE_CRASHED 				SetBit(2)
#define REPORT_KEYLESS_MISSING			SetBit(3)

// Command Code List
#define CMD_CODE_GEN								0
#define CMD_CODE_REPORT							1
#define CMD_CODE_AUDIO							2
#define CMD_CODE_FINGER							3
#define CMD_CODE_HMI2								4

// Command Sub-Code List
#define CMD_GEN_INFO								0
#define CMD_GEN_LED									1

#define CMD_REPORT_RTC							0
#define CMD_REPORT_ODOM							1
#define CMD_REPORT_UNITID						2

#define CMD_AUDIO_BEEP							0
#define CMD_AUDIO_MUTE							1
#define CMD_AUDIO_VOL								2

#define CMD_FINGER_ADD							0
#define CMD_FINGER_DEL							1
#define CMD_FINGER_RST							2

#define CMD_HMI2_SHUTDOWN				0

// Response Status List
#define RESPONSE_STATUS_ERROR				0
#define RESPONSE_STATUS_OK					1
#define RESPONSE_STATUS_INVALID			2

#endif /* CONFIG_H_ */
