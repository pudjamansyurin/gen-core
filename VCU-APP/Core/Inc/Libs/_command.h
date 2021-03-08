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

/* Exported define ------------------------------------------------------------*/
// Response Status List
#define RESPONSE_STATUS_ERROR         (uint8_t) 0
#define RESPONSE_STATUS_OK            (uint8_t) 1
#define RESPONSE_STATUS_INVALID       (uint8_t) 2

// Command Code List
#define CMD_CODE_GEN                  (uint8_t) 0
#define CMD_CODE_REPORT               (uint8_t) 1
#define CMD_CODE_AUDIO                (uint8_t) 2
#define CMD_CODE_FINGER               (uint8_t) 3
#define CMD_CODE_REMOTE               (uint8_t) 4
#define CMD_CODE_FOTA                 (uint8_t) 5
#define CMD_CODE_NET                  (uint8_t) 6

// Command Sub-Code List
#define CMD_GEN_INFO                  (uint8_t) 0
#define CMD_GEN_LED                   (uint8_t) 1
#define CMD_GEN_OVERRIDE              (uint8_t) 2

#define CMD_REPORT_RTC                (uint8_t) 0
#define CMD_REPORT_ODOM               (uint8_t) 1

#define CMD_AUDIO_BEEP                (uint8_t) 0
#define CMD_AUDIO_MUTE                (uint8_t) 1

#define CMD_FINGER_FETCH              (uint8_t) 0
#define CMD_FINGER_ADD                (uint8_t) 1
#define CMD_FINGER_DEL                (uint8_t) 2
#define CMD_FINGER_RST                (uint8_t) 3

#define CMD_REMOTE_PAIRING            (uint8_t) 0
#define CMD_REMOTE_UNITID             (uint8_t) 1

#define CMD_FOTA_VCU                  (uint8_t) 0
#define CMD_FOTA_HMI                  (uint8_t) 1

#define CMD_NET_SEND_USSD             (uint8_t) 0
#define CMD_NET_READ_SMS              (uint8_t) 1


/* Public functions implementation --------------------------------------------*/
uint8_t CMD_ValidateCommand(void *ptr, uint8_t len);
void CMD_ExecuteCommand(command_t* cmd);
void CMD_GenInfo(response_t *resp, uint8_t *hmi_started, uint16_t *hmi_version);
void CMD_GenLed(command_t *cmd);
void CMD_GenOverride(command_t *cmd, uint8_t *override_state);
void CMD_Fota(response_t *resp, IAP_TYPE type, uint16_t *bat, uint16_t *hmi_version);
void CMD_ReportRTC(command_t *cmd);
void CMD_ReportOdom(command_t *cmd);
void CMD_AudioBeep(osThreadId_t threadId);
void CMD_AudioMute(command_t *cmd, osThreadId_t threadId);
void CMD_FingerAdd(response_t *resp, osMessageQueueId_t queue);
void CMD_FingerFetch(response_t *resp, osMessageQueueId_t queue);
void CMD_FingerDelete(response_t *resp, command_t *cmd, osThreadId_t threadId, osMessageQueueId_t queue);
void CMD_Finger(response_t *resp);
void CMD_RemoteUnitID(command_t *cmd, osThreadId_t threadIot, osThreadId_t threadRemote);
void CMD_RemotePairing(response_t *resp);
void CMD_NetQuota(response_t *resp, osMessageQueueId_t queue);

#endif /* INC_LIBS__COMMAND_H_ */
