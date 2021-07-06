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
#define FTP_PASS "@2,TUST4W9#O"

/* Public functions implementation
 * --------------------------------------------*/
void SIMCon_Init(void) {
	if (EEPROM.active)
		SIMCon_ReadStore();
	else SIMCon_WriteStore();
}

void SIMCon_ReadStore(void) {
	SIMCon_ApnStore(NULL);
	SIMCon_FtpStore(NULL);
	SIMCon_MqttStore(NULL);
}

uint8_t SIMCon_WriteStore(void) {
  uint8_t ok = 0;
  con_apn_t apn = {
		  .name = APN_NAME,
		  .user = APN_USER,
		  .pass = APN_PASS,
  };
  con_ftp_t ftp = {
		  .host = FTP_HOST,
		  .user = FTP_USER,
		  .pass = FTP_PASS,
  };
  con_mqtt_t mqtt = {
		  .host = MQTT_HOST,
		  .port = MQTT_PORT,
		  .user = MQTT_USER,
		  .pass = MQTT_PASS,
  };

  ok += SIMCon_ApnStore(&apn);
  ok += SIMCon_FtpStore(&ftp);
  ok += SIMCon_MqttStore(&mqtt);

  return ok == 3;
}

uint8_t SIMCon_ApnStore(con_apn_t *s) {
  con_apn_t *d = &SIM.con.apn;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_APN_NAME, s == NULL ? NULL : s->name, d->name);
  ok += EE_Cmd(VA_APN_USER, s == NULL ? NULL : s->user, d->user);
  ok += EE_Cmd(VA_APN_PASS, s == NULL ? NULL : s->pass, d->pass);

  return ok == 3;
}

uint8_t SIMCon_FtpStore(con_ftp_t *s) {
  con_ftp_t *d = &SIM.con.ftp;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_FTP_HOST, s == NULL ? NULL : s->host, d->host);
  ok += EE_Cmd(VA_FTP_USER, s == NULL ? NULL : s->user, d->user);
  ok += EE_Cmd(VA_FTP_PASS, s == NULL ? NULL : s->pass, d->pass);

  return ok == 3;
}

uint8_t SIMCon_MqttStore(con_mqtt_t *s) {
  con_mqtt_t* d = &SIM.con.mqtt;
  uint8_t ok = 0;

  ok += EE_Cmd(VA_MQTT_HOST, s == NULL ? NULL : s->host, d->host);
  ok += EE_Cmd(VA_MQTT_PORT, s == NULL ? NULL : &s->port, &d->port);
  ok += EE_Cmd(VA_MQTT_USER, s == NULL ? NULL : s->user, d->user);
  ok += EE_Cmd(VA_MQTT_PASS, s == NULL ? NULL : s->pass, d->pass);

  return ok == 4;
}
