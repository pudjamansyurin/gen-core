/*
 * _sim_net.c
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */


/* Includes
 * --------------------------------------------*/
#include "Drivers/_sim_net.h"
#include "Drivers/_simcom.h"
#include "Libs/_eeprom.h"

/* Public functions implementation
 * --------------------------------------------*/
void SIM_NET_LoadStore(void) {
	SIM_NET_ConStore(NULL, NULL, NULL);
	SIM_NET_FtpStore(NULL, NULL, NULL);
	SIM_NET_MqttStore(NULL, NULL, NULL, NULL);
}

uint8_t SIM_NET_ConStore(char* apn, char* user, char *pass) {
	net_con_t *dst = &SIM.net.con;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_NET_CON_APN, apn, &dst->apn, sizeof(dst->apn));
  ok += EE_Cmd(VA_NET_CON_USER, user, &dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_NET_CON_PASS, pass, &dst->pass, sizeof(dst->pass));

  return ok == 3;
}

uint8_t SIM_NET_FtpStore(char* host, char* user, char *pass) {
  net_ftp_t* dst = &SIM.net.ftp;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_NET_FTP_HOST, host, dst->host, sizeof(dst->host));
  ok += EE_Cmd(VA_NET_FTP_USER, user, dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_NET_FTP_PASS, pass, dst->pass, sizeof(dst->pass));

  return ok == 3;
}

uint8_t SIM_NET_MqttStore(char* host, uint16_t *port, char* user, char *pass) {
  net_mqtt_t* dst = &SIM.net.mqtt;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_NET_MQTT_HOST, host, dst->host, sizeof(dst->host));
  ok += EE_Cmd(VA_NET_MQTT_PORT, port, &dst->port, sizeof(dst->port));
  ok += EE_Cmd(VA_NET_MQTT_USER, user, dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_NET_MQTT_PASS, pass, dst->pass, sizeof(dst->pass));

  return ok == 4;
}
