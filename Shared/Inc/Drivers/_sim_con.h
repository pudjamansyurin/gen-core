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

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  char name[20];
  char user[20];
  char pass[20];
} con_apn_t;

typedef struct {
  char host[30];
  uint16_t port;
  char user[30];
  char pass[30];
} con_mqtt_t;

typedef struct {
  char host[30];
  char user[30];
  char pass[30];
} con_ftp_t;

typedef struct {
  con_apn_t apn;
  con_ftp_t ftp;
  con_mqtt_t mqtt;
} sim_con_t;

/* Public functions prototype
 * --------------------------------------------*/
void SIMCon_LoadStore(void);
uint8_t SIMCon_ApnStore(char* name, char* user, char *pass);
uint8_t SIMCon_FtpStore(char* host, char* user, char *pass);
uint8_t SIMCon_MqttStore(char* host, uint16_t *port, char* user, char *pass);

#endif /* INC_DRIVERS__SIMCon_H_ */
