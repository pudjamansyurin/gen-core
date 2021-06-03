/*
 * _command.h
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

#ifndef INC_LIBS__COMMAND_H_
#define INC_LIBS__COMMAND_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_reporter.h"

#define CMD_SUB_MAX ((uint8_t) 10)

/* Exported enums
 * ------------------------------------------------------------*/
typedef enum {
	RESP_ERROR = 0,
	RESP_OK,
	RESP_INVALID,
} CMD_RESP;

typedef enum {
	CMD_CODE_GEN = 0,
	CMD_CODE_OVD,
	CMD_CODE_AUDIO,
	CMD_CODE_FGR,
	CMD_CODE_RMT,
	CMD_CODE_FOTA,
	CMD_CODE_NET,
	CMD_CODE_HBAR,
	CMD_CODE_MCU,
	CMD_CODE_MAX,
} CMD_CODE;

typedef enum {
	CMD_GEN_INFO = 0,
	CMD_GEN_LED,
	CMD_GEN_RTC,
	CMD_GEN_ODOM,
	CMD_GEN_DETECTOR,
	CMD_GEN_RPT_FLUSH,
	CMD_GEN_RPT_BLOCK,
	CMD_GEN_MAX,
} CMD_SUB_GEN;

typedef enum {
	CMD_OVD_STATE = 0,
	CMD_OVD_RPT_INTERVAL,
	CMD_OVD_RPT_FRAME,
	CMD_OVD_RMT_SEAT,
	CMD_OVD_RMT_ALARM,
	CMD_OVD_MAX,
} CMD_SUB_OVD;

typedef enum {
	CMD_AUDIO_BEEP = 0,
	CMD_AUDIO_MAX,
} CMD_SUB_AUDIO;

typedef enum {
	CMD_FGR_FETCH = 0,
	CMD_FGR_ADD,
	CMD_FGR_DEL,
	CMD_FGR_RST,
	CMD_FGR_MAX,
} CMD_SUB_FGR;

typedef enum {
	CMD_RMT_PAIRING = 0,
	CMD_RMT_MAX,
} CMD_SUB_RMT;

typedef enum {
	CMD_FOTA_VCU = 0,
	CMD_FOTA_HMI,
	CMD_FOTA_MAX,
} CMD_SUB_FOTA;

typedef enum {
	CMD_NET_SEND_USSD = 0,
	CMD_NET_READ_SMS,
	CMD_NET_MAX,
} CMD_SUB_NET;

typedef enum {
	CMD_HBAR_DRIVE = 0,
	CMD_HBAR_TRIP,
	CMD_HBAR_REPORT,
	CMD_HBAR_REVERSE,
	CMD_HBAR_MAX,
} CMD_SUB_HBAR;

typedef enum {
	CMD_MCU_SPEED_MAX = 0,
	CMD_MCU_TEMPLATES,
	CMD_MCU_MAX,
} CMD_SUB_MCU;

/* Public functions implementation
 * --------------------------------------------*/
void CMD_Init(void);
uint8_t CMD_Validate(command_t *cmd);
uint8_t CMD_ValidateRaw(void *ptr, uint8_t len);
void CMD_Execute(command_t *cmd);
void CMD_GenInfo(response_t *resp);
void CMD_ReportRTC(command_t *cmd);
void CMD_FingerAdd(response_t *resp, osMessageQueueId_t queue);
void CMD_FingerFetch(response_t *resp);
void CMD_Finger(response_t *resp);
void CMD_RemotePairing(response_t *resp);
void CMD_NetQuota(response_t *resp, osMessageQueueId_t queue);

#endif /* INC_LIBS__COMMAND_H_ */
