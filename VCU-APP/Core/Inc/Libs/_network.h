/*
 * _network.h
 *
 *  Created on: Jun 6, 2021
 *      Author: pujak
 */

#ifndef INC_LIBS__NETWORK_H_
#define INC_LIBS__NETWORK_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions declaration -------------------------------------------*/
void NET_CheckClock(void);
void NET_CheckCommand(void);
void NET_CheckPayload(PAYLOAD_TYPE type);
uint8_t NET_SendUSSD(void);
uint8_t NET_ReadSMS(void);

#endif /* INC_LIBS__NETWORK_H_ */
