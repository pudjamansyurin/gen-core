/*
 * _command.h
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

#ifndef INC_LIBS__COMMAND_H_
#define INC_LIBS__COMMAND_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Libs/_utils.h"

/* Exported constants
 * --------------------------------------------*/
#define CMD_SUB_MAX ((uint8_t)10)

/* Exported enums
 * --------------------------------------------*/
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

/* Exported structs
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
  char message[200];
} response_data_t;

typedef struct __attribute__((packed)) {
  command_header_t header;
  response_data_t data;
} response_t;

typedef struct __attribute__((packed)) {
  command_header_t header;
  struct __attribute__((packed)) {
    char value[200];
  } data;
} command_t;

/* Public functions prototype
 * --------------------------------------------*/
void CMD_Init(void);
uint8_t CMD_ValidateCode(command_t* cmd);
uint8_t CMD_ValidateContent(void* ptr, uint8_t len);
void CMD_Execute(command_t* cmd);

#endif /* INC_LIBS__COMMAND_H_ */
