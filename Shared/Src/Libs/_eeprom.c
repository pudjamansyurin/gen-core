/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_eeprom.h"

#include "Drivers/_ee24xx.h"
#include "i2c.h"

#if (APP)
#include "Drivers/_aes.h"
#include "Libs/_hbar.h"
#endif

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t EepromRecMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static I2C_HandleTypeDef* pi2c = &hi2c2;

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t EE_Init(void) {
  uint8_t ok;

  lock();
  printf("EEPROM:Init\n");
  EEPROM24XX_SetDevice(pi2c, EE_ADDR);
  ok = EEPROM24XX_IsConnected(100);

  if (ok) {
#if (APP)
  	AES_KeyStore(NULL);
    HBAR_ModeStore(NULL);
    for (uint8_t m = 0; m < HBAR_M_MAX; m++)
    	HBAR_SubModeStore(m, NULL);
    for (uint8_t mTrip = 0; mTrip < HBAR_M_TRIP_MAX; mTrip++)
    	HBAR_TripMeterStore(mTrip, NULL);

    IAP_VersionStore(NULL);
#endif
    IAP_TypeStore(NULL);
  } else
    printf("EEPROM:Error\n");
  unlock();

  return ok;
}

uint8_t EE_Cmd(EE_VA va, void* src, void *dst, uint16_t size) {
  uint16_t addr = EE_WORD(va);
  uint8_t ok;

  lock();
  if (src == NULL) {
    ok = EEPROM24XX_Load(addr, dst, size);
  } else {
  	memcpy(dst, src, size);
    ok = EEPROM24XX_Save(addr, src, size);
  }
  unlock();

  return ok;
}

//uint8_t EE_NetCon(EE_CMD cmd, net_con_t con) {
//  net_con_t* d = &SIM.net.con;
//  uint8_t ok = 0;
//
//  ok += EE_Cmd(VA_NET_CON_APN, cmd, con.apn, d->apn, sizeof(d->apn));
//  ok += EE_Cmd(VA_NET_CON_USER, cmd, con.user, d->user, sizeof(d->user));
//  ok += EE_Cmd(VA_NET_CON_PASS, cmd, con.pass, d->pass, sizeof(d->pass));
//
//  return ok == 3;
//}
//
//uint8_t EE_NetFtp(EE_CMD cmd, net_ftp_t ftp) {
//  net_ftp_t* d = &SIM.net.ftp;
//  uint8_t ok = 0;
//
//  ok += EE_Cmd(VA_NET_FTP_HOST, cmd, ftp.host, d->host, sizeof(d->host));
//  ok += EE_Cmd(VA_NET_FTP_USER, cmd, ftp.user, d->user, sizeof(d->user));
//  ok += EE_Cmd(VA_NET_FTP_PASS, cmd, ftp.pass, d->pass, sizeof(d->pass));
//
//  return ok == 3;
//}
//
//uint8_t EE_NetMqtt(EE_CMD cmd, net_mqtt_t mqtt) {
//  net_mqtt_t* d = &SIM.net.mqtt;
//  uint8_t ok = 0;
//
//  ok += EE_Cmd(VA_NET_MQTT_HOST, cmd, mqtt.host, d->host, sizeof(d->host));
//  ok += EE_Cmd(VA_NET_MQTT_PORT, cmd, &(mqtt.port), &(d->port), sizeof(d->port));
//  ok += EE_Cmd(VA_NET_MQTT_USER, cmd, mqtt.user, d->user, sizeof(d->user));
//  ok += EE_Cmd(VA_NET_MQTT_PASS, cmd, mqtt.pass, d->pass, sizeof(d->pass));
//
//  return ok == 4;
//}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(EepromRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(EepromRecMutexHandle);
#endif
}
