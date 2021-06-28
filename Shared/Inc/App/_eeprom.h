/*
 * _flash.h
 *
 *  Created on: Sep 9, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"
#include "Drivers/_simcom.h"

#if (APP)
#include "Libs/_hbar.h"
#endif

/* Exported macros
 * --------------------------------------------*/
#define EE_ADDR ((uint16_t)0xA0)
#define EE_NULL ((uint8_t)0)
#define EE_WORD(X) ((uint16_t)(X * 32))

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint32_t FLAG;
  uint16_t VERSION;
  IAP_TYPE TYPE;
} fota_t;

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  VA_RESET,
  VA_UNUSED,
  VA_AES_KEY,
  VA_FOTA_VERSION,
  VA_FOTA_FLAG,
  VA_FOTA_TYPE,
  VA_TRIP_A,
  VA_TRIP_B,
  VA_TRIP_ODO,
  VA_MODE_DRIVE,
  VA_MODE_TRIP,
  VA_MODE_PREDICTION,
  VA_MODE,
  VA_NET_CON_APN,
  VA_NET_CON_USER,
  VA_NET_CON_PASS,
  VA_NET_FTP_HOST,
  VA_NET_FTP_USER,
  VA_NET_FTP_PASS,
  VA_NET_MQTT_HOST,
  VA_NET_MQTT_PORT,
  VA_NET_MQTT_USER,
  VA_NET_MQTT_PASS,
  VA_MAX,
} EE_VA;

typedef enum { EE_CMD_R = 0, EE_CMD_W = 1 } EE_CMD;

/* Exported variables
 * --------------------------------------------*/
extern fota_t FOTA;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t EE_Init(void);
#if (APP)
uint8_t EE_AesKey(EE_CMD cmd, uint32_t val[4]);
uint8_t EE_TripMeter(EE_CMD cmd, HBAR_MODE_TRIP mTrip, uint16_t val);
uint8_t EE_SubMode(EE_CMD cmd, HBAR_MODE m, uint8_t val);
uint8_t EE_Mode(EE_CMD cmd, uint8_t val);
#endif
uint8_t EE_FotaType(EE_CMD cmd, IAP_TYPE val);
uint8_t EE_FotaFlag(EE_CMD cmd, uint32_t val);
uint8_t EE_FotaVersion(EE_CMD cmd, uint16_t val);
//uint8_t EE_NetCon(EE_CMD cmd, net_con_t con);
//uint8_t EE_NetFtp(EE_CMD cmd, net_ftp_t ftp);
//uint8_t EE_NetMqtt(EE_CMD cmd, net_mqtt_t mqtt);
#endif /* EEPROM_H_ */
