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

/* Exported constants
 * --------------------------------------------*/
#define EE_STR_MAX (30)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  VA_AES_KEY,
  VA_IAP_VERSION,
  VA_IAP_FLAG,
  VA_IAP_TYPE,
  VA_TRIP_A,
  VA_TRIP_B,
  VA_TRIP_ODO,
  VA_MODE_DRIVE,
  VA_MODE_TRIP,
  VA_MODE_AVG,
  VA_MODE,
  VA_APN_NAME,
  VA_APN_USER,
  VA_APN_PASS,
  VA_FTP_HOST,
  VA_FTP_USER,
  VA_FTP_PASS,
  VA_MQTT_HOST,
  VA_MQTT_PORT,
  VA_MQTT_USER,
  VA_MQTT_PASS,
  VA_MAX,
} EE_VA;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t EE_Init(void);
void EE_Refresh(void);
uint8_t EE_Cmd(EE_VA va, const void *src, void *dst);
uint8_t EE_IO_Active(void);
uint8_t EE_IO_Used(void);
#endif /* EEPROM_H_ */
