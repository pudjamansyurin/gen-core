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
typedef char con_string_t[EE_STR_MAX];

typedef struct {
	con_string_t name;
	con_string_t user;
	con_string_t pass;
} con_apn_t;

typedef struct {
	con_string_t host;
  uint16_t port;
	con_string_t user;
	con_string_t pass;
} con_mqtt_t;

typedef struct {
	con_string_t host;
	con_string_t user;
	con_string_t pass;
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

const con_ftp_t* SIMCon_IO_Ftp(void);
const con_mqtt_t* SIMCon_IO_Mqtt(void);
const con_apn_t* SIMCon_IO_Apn(void);
void SIMCon_IO_SetApn(const con_apn_t *apn);
#endif /* INC_DRIVERS__SIMCon_H_ */
