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

/* Private constants
 * --------------------------------------------*/
#define APN_NAME "3gprs"
#define APN_USER "3gprs"
#define APN_PASS "3gprs"
//#define APN_NAME                             "telkomsel"
//#define APN_USER                        "wap"
//#define APN_PASS                        "wap123"
//#define APN_NAME                             "indosatgprs"
//#define APN_USER                        "indosat"
//#define APN_PASS                        "indosat"

#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT ((uint16_t)1883)
#define MQTT_USER ""
#define MQTT_PASS ""
//#define MQTT_HOST                          "pujakusumae-30856.portmap.io"
//#define MQTT_PORT                 ((uint16_t)46606)
//#define MQTT_HOST                          "mqtt.eclipseprojects.io"
//#define MQTT_PORT                 ((uint16_t)1883)

#define FTP_HOST "ftp.garda-energi.com"
#define FTP_USER "fota@garda-energi.com"
#define FTP_PASS "@Garda313"

/* Public functions implementation
 * --------------------------------------------*/
void SIMCon_LoadStore(void) {
	SIMCon_ApnStore(NULL, NULL, NULL);
	SIMCon_FtpStore(NULL, NULL, NULL);
	SIMCon_MqttStore(NULL, NULL, NULL, NULL);
}

void SIMCon_SetDefaultStore(void) {
  uint16_t mqtt_port = MQTT_PORT;

  SIMCon_MqttStore(MQTT_HOST, &mqtt_port, MQTT_USER, MQTT_PASS);
  SIMCon_ApnStore(APN_NAME, APN_USER, APN_PASS);
  SIMCon_FtpStore(FTP_HOST, FTP_USER, FTP_PASS);
}

uint8_t SIMCon_ApnStore(char* name, char* user, char *pass) {
	con_apn_t *dst = &SIM.con.apn;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_APN_NAME, name, &dst->name, sizeof(dst->name));
  ok += EE_Cmd(VA_APN_USER, user, &dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_APN_PASS, pass, &dst->pass, sizeof(dst->pass));

  return ok == 3;
}

uint8_t SIMCon_FtpStore(char* host, char* user, char *pass) {
  con_ftp_t* dst = &SIM.con.ftp;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_FTP_HOST, host, dst->host, sizeof(dst->host));
  ok += EE_Cmd(VA_FTP_USER, user, dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_FTP_PASS, pass, dst->pass, sizeof(dst->pass));

  return ok == 3;
}

uint8_t SIMCon_MqttStore(char* host, uint16_t *port, char* user, char *pass) {
  con_mqtt_t* dst = &SIM.con.mqtt;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_MQTT_HOST, host, dst->host, sizeof(dst->host));
  ok += EE_Cmd(VA_MQTT_PORT, port, &dst->port, sizeof(dst->port));
  ok += EE_Cmd(VA_MQTT_USER, user, dst->user, sizeof(dst->user));
  ok += EE_Cmd(VA_MQTT_PASS, pass, dst->pass, sizeof(dst->pass));

  return ok == 4;
}
