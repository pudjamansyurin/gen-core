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

/* Exported macros
 * --------------------------------------------*/
#define EE_ADDR ((uint16_t)0xA0)
#define EE_WORD(X) ((uint16_t)(X * 32))

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  VA_RESET,
  VA_UNUSED,
  VA_AES_KEY,
  VA_IAP_VERSION,
  VA_IAP_FLAG,
  VA_IAP_TYPE,
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

/* Public functions prototype
 * --------------------------------------------*/
uint8_t EE_Init(void);
uint8_t EE_Cmd(EE_VA va, void *src, void *dst, uint16_t size);
#endif /* EEPROM_H_ */
