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
void CMD_GenInfo(response_t *resp);
void CMD_GenLed(command_t *cmd);
void CMD_GenOverride(command_t *cmd, uint8_t *override_state);
void CMD_GenFota(IAP_TYPE type, response_t *resp, uint16_t *bat, uint16_t *hmi_version);
void CMD_ReportRTC(command_t *cmd, datetime_t *rtc);
void CMD_ReportOdom(command_t *cmd);
void CMD_ReportUnitID(command_t *cmd);
void CMD_AudioBeep(void);
void CMD_AudioMute(command_t *cmd);
void CMD_Finger(uint8_t event, response_t *resp);
void CMD_RemotePairing(response_t *resp);

#endif /* INC_LIBS__COMMAND_H_ */
