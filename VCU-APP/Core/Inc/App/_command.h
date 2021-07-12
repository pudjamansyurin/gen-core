/*
 * _command.h
 *
 *  Created on: 28 Oct 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__COMMAND_H_
#define INC_APP__COMMAND_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_rtc.h"

/* Exported constants
 * --------------------------------------------*/
#define CMD_MSG_MAX 200

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  CMDR_ERROR = 0,
  CMDR_OK,
  CMDR_INVALID,
} CMD_RESP;

typedef enum {
  CMDC_GEN = 0,
  CMDC_OVD,
  CMDC_AUDIO,
  CMDC_FGR,
  CMDC_RMT,
  CMDC_FOTA,
  CMDC_NET,
  CMDC_CON,
  CMDC_HBAR,
  CMDC_MCU,
  CMDC_MAX,
} CMD_CODE;

typedef enum {
  CMD_GEN_INFO = 0,
  CMD_GEN_LED,
  CMD_GEN_RTC,
  CMD_GEN_ODO,
  CMD_GEN_ANTITHIEF,
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
  CMD_CON_APN = 0,
  CMD_CON_FTP,
  CMD_CON_MQTT,
  CMD_CON_MAX,
} CMD_SUB_CON;

typedef enum {
  CMD_HBAR_DRIVE = 0,
  CMD_HBAR_TRIP,
  CMD_HBAR_AVG,
  CMD_HBAR_REVERSE,
  CMD_HBAR_MAX,
} CMD_SUB_HBAR;

typedef enum {
  CMD_MCU_SPEED_MAX = 0,
  CMD_MCU_TEMPLATES,
  CMD_MCU_MAX,
} CMD_SUB_MCU;

/* Exported types
 * --------------------------------------------*/
typedef struct __attribute__((packed)) {
  char prefix[2];
  uint8_t size;
  uint32_t vin;
  datetime_t send_time;
  uint8_t code;
  uint8_t sub_code;
} command_header_t;

typedef struct __attribute__((packed)) {
  uint8_t res_code;
  char message[CMD_MSG_MAX];
} response_data_t;

typedef struct __attribute__((packed)) {
  command_header_t header;
  response_data_t data;
} response_t;

typedef struct __attribute__((packed)) {
  command_header_t header;
  struct __attribute__((packed)) {
    char value[CMD_MSG_MAX];
  } data;
} command_t;

/* Public functions prototype
 * --------------------------------------------*/
void CMD_Init(void);
bool CMD_ValidateCode(const command_t* cmd);
bool CMD_ValidateContent(const void* ptr, uint8_t len);
uint8_t CMD_GetPayloadSize(const command_t* cmd);
void CMD_Execute(const command_t* cmd);

#endif /* INC_APP__COMMAND_H_ */
