/*
 * _network.h
 *
 *  Created on: Jun 6, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__NETWORK_H_
#define INC_APP__NETWORK_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"
#include "App/_reporter.h"

/* Public functions prototype
 * --------------------------------------------*/
void NET_CheckClock(void);
void NET_CheckCommand(void);
void NET_CheckPayload(PAYLOAD_TYPE type);
bool NET_SendUSSD(void);
bool NET_ReadSMS(void);

#endif /* INC_APP__NETWORK_H_ */
