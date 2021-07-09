/*
 * simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__SIMCOM_H_
#define INC_DRIVERS__SIMCOM_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"
#include "DMA/_dma_simcom.h"
#include "Drivers/_sim_con.h"
#include "Drivers/_sim_state.h"

#if (APP)
#include "Drivers/_rtc.h"
#endif

/* Public functions prototype
 * --------------------------------------------*/
void SIM_Init(void);
void SIM_DeInit(void);
uint8_t SIM_BatSufficient(void);
void SIM_Lock(void);
void SIM_Unlock(void);
uint8_t SIM_SetState(SIM_STATE state, uint32_t timeout);
char* SIM_Resp(const char* keyword, const char* from);
SIMR SIM_Cmd(const char* command, char* reply, uint32_t ms);
#if (APP)
uint8_t SIM_FetchTime(timestamp_t* ts);
uint8_t SIM_SendUSSD(const char* ussd, char* buf, uint8_t buflen);
uint8_t SIM_ReadLastSMS(char* buf, uint8_t buflen);
uint8_t SIM_Upload(const void* payload, uint16_t size);
int SIM_GetData(unsigned char* buf, int count);
uint8_t SIM_GotResponse(uint32_t timeout);
#endif
#endif /* INC_DRIVERS__SIMCOM_H_ */
