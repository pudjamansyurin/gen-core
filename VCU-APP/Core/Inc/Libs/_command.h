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
void CMD_CheckCommand(command_t command);
void CMD_GenInfo(response_t *resp, uint8_t *hmi_started, uint16_t *hmi_version);
void CMD_GenLed(command_t *cmd);
void CMD_GenOverride(command_t *cmd, uint8_t *override_state);
void CMD_GenFota(IAP_TYPE type, response_t *resp, uint16_t *bat, uint16_t *hmi_version);
void CMD_ReportRTC(command_t *cmd);
void CMD_ReportOdom(command_t *cmd);
void CMD_ReportUnitID(command_t *cmd);
void CMD_AudioBeep(osThreadId_t threadId);
void CMD_AudioMute(osThreadId_t threadId, command_t *cmd);
void CMD_FingerAdd(osThreadId_t threadId, osMessageQueueId_t queue, response_t *resp);
void CMD_FingerFetch(osThreadId_t threadId, osMessageQueueId_t queue, response_t *resp);
void CMD_Finger(osThreadId_t threadId, uint8_t event, response_t *resp);
void CMD_RemotePairing(osThreadId_t threadId, response_t *resp);

#endif /* INC_LIBS__COMMAND_H_ */
