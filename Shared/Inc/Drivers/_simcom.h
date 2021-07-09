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

/* Exported constants
 * --------------------------------------------*/
#define NET_BOOT_MS ((uint16_t)8000)
#define NET_GUARD_MS ((uint16_t)1000)

/* Exported types
 * --------------------------------------------*/
typedef struct {
  uint8_t signal;
  SIM_STATE state;
  AT_CIPSTATUS ipstatus;
  char* response;
} sim_data_t;

typedef struct {
  sim_data_t d;
  sim_con_t con;
  UART_HandleTypeDef* puart;
  DMA_HandleTypeDef* pdma;
} sim_t;

/* Exported variables
 * --------------------------------------------*/
extern sim_t SIM;

/* Public functions prototype
 * --------------------------------------------*/
void SIM_Init(void);
void SIM_DeInit(void);
void SIM_Lock(void);
void SIM_Unlock(void);
uint8_t SIM_SetState(SIM_STATE state, uint32_t timeout);
char* SIM_Resp(char* keyword, char* from);
SIMR SIM_Cmd(char* command, char* reply, uint32_t ms);
#if (APP)
uint8_t SIM_FetchTime(timestamp_t* ts);
uint8_t SIM_SendUSSD(char* ussd, char* buf, uint8_t buflen);
uint8_t SIM_ReadNewSMS(char* buf, uint8_t buflen);
// uint8_t SIM_CheckQuota(char *buf, uint8_t buflen);
uint8_t SIM_Upload(void* payload, uint16_t size);
int SIM_GetData(unsigned char* buf, int count);
uint8_t SIM_GotResponse(uint32_t timeout);
#endif

#endif /* INC_DRIVERS__SIMCOM_H_ */
