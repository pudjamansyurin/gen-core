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

/* Public functions implementation --------------------------------------------*/
void CMD_Init(osMessageQueueId_t mCmdQueue);
void CMD_CheckCommand(command_t* cmd);
void CMD_GenInfo(response_t *resp, uint8_t *hmi_started, uint16_t *hmi_version);
void CMD_GenLed(command_t *cmd);
void CMD_GenOverride(command_t *cmd, uint8_t *override_state);
void CMD_Fota(response_t *resp, IAP_TYPE type, uint16_t *bat, uint16_t *hmi_version);
void CMD_ReportRTC(command_t *cmd);
void CMD_ReportOdom(command_t *cmd);
void CMD_AudioBeep(osThreadId_t threadId);
void CMD_AudioMute(command_t *cmd, osThreadId_t threadId);
void CMD_FingerAdd(response_t *resp, osThreadId_t threadId, osMessageQueueId_t queue);
void CMD_FingerFetch(response_t *resp, osThreadId_t threadId, osMessageQueueId_t queue);
void CMD_Finger(response_t *resp, osThreadId_t threadId, uint8_t event);
void CMD_RemoteUnitID(command_t *cmd, osThreadId_t threadIot, osThreadId_t threadRemote);
void CMD_RemotePairing(response_t *resp, osThreadId_t threadId);

#endif /* INC_LIBS__COMMAND_H_ */
