/*
 * _sim_net.h
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

#ifndef INC_DRIVERS__SIM_NET_H_
#define INC_DRIVERS__SIM_NET_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  char apn[20];
  char user[20];
  char pass[20];
} net_con_t;

typedef struct {
  char host[30];
  uint16_t port;
  char user[30];
  char pass[30];
} net_mqtt_t;

typedef struct {
  char host[30];
  char user[30];
  char pass[30];
} net_ftp_t;

typedef struct {
  net_con_t con;
  net_ftp_t ftp;
  net_mqtt_t mqtt;
} sim_net_t;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t SIM_NET_ConStore(char* apn, char* user, char *pass);
uint8_t SIM_NET_Ftp(char* host, char* user, char *pass);
uint8_t SIM_NET_Mqtt(char* host, uint16_t *port, char* user, char *pass);

#endif /* INC_DRIVERS__SIM_NET_H_ */
