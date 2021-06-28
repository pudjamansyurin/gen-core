/*
 * _sim_con.c
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */


/* Includes
 * --------------------------------------------*/
#include "Drivers/_sim_con.h"
#include "Drivers/_simcom.h"
#include "Libs/_eeprom.h"

/* Public functions implementation
 * --------------------------------------------*/
void SIMCon_LoadStore(void) {
	SIMCon_ApnStore(NULL, NULL, NULL);
	SIMCon_FtpStore(NULL, NULL, NULL);
	SIMCon_MqttStore(NULL, NULL, NULL, NULL);
}

uint8_t SIMCon_ApnStore(char* name, char* user, char *pass) {
	con_apn_t *dst = &SIM.con.apn;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_NET_APN_NAME, name, &dst->name, sizeof(dst->name));
  ok += EE_Cmd(VA_NET_APN_USER, user, &dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_NET_APN_PASS, pass, &dst->pass, sizeof(dst->pass));

  return ok == 3;
}

uint8_t SIMCon_FtpStore(char* host, char* user, char *pass) {
  con_ftp_t* dst = &SIM.con.ftp;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_NET_FTP_HOST, host, dst->host, sizeof(dst->host));
  ok += EE_Cmd(VA_NET_FTP_USER, user, dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_NET_FTP_PASS, pass, dst->pass, sizeof(dst->pass));

  return ok == 3;
}

uint8_t SIMCon_MqttStore(char* host, uint16_t *port, char* user, char *pass) {
  con_mqtt_t* dst = &SIM.con.mqtt;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_NET_MQTT_HOST, host, dst->host, sizeof(dst->host));
  ok += EE_Cmd(VA_NET_MQTT_PORT, port, &dst->port, sizeof(dst->port));
  ok += EE_Cmd(VA_NET_MQTT_USER, user, dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_NET_MQTT_PASS, pass, dst->pass, sizeof(dst->pass));

  return ok == 4;
}
