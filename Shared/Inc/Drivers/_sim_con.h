/*
 * _sim_con.h
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

#ifndef INC_DRIVERS__SIMCon_H_
#define INC_DRIVERS__SIMCon_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"
#include "Libs/_eeprom.h"

/* Exported types
 * --------------------------------------------*/
typedef struct {
  char name[EE_STR_MAX];
  char user[EE_STR_MAX];
  char pass[EE_STR_MAX];
} con_apn_t;

typedef struct {
  char host[EE_STR_MAX];
  uint16_t port;
  char user[EE_STR_MAX];
  char pass[EE_STR_MAX];
} con_mqtt_t;

typedef struct {
  char host[EE_STR_MAX];
  char user[EE_STR_MAX];
  char pass[EE_STR_MAX];
} con_ftp_t;

typedef struct {
  con_apn_t apn;
  con_ftp_t ftp;
  con_mqtt_t mqtt;
} sim_con_t;

/* Public functions prototype
 * --------------------------------------------*/
void SIMCon_Init(void);
void SIMCon_EE_Read(void);
uint8_t SIMCon_EE_Write(void);
uint8_t SIMCon_EE_Apn(con_apn_t *s);
uint8_t SIMCon_EE_Ftp(con_ftp_t *s);
uint8_t SIMCon_EE_Mqtt(con_mqtt_t *s);

#endif /* INC_DRIVERS__SIMCon_H_ */
