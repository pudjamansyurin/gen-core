/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_eeprom.h"

#include "Drivers/_ee24xx.h"
#include "i2c.h"

#if (APP)
#include "App/_reporter.h"
#include "Drivers/_aes.h"
#include "Libs/_hbar.h"
#include "Libs/_remote.h"
#include "Nodes/VCU.h"

#endif

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t EepromRecMutexHandle;
#endif

/* Public variables
 * --------------------------------------------*/
fota_t FOTA = {0};

/* Private variables
 * --------------------------------------------*/
static I2C_HandleTypeDef* pi2c = &hi2c2;

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t Cmd(EE_CMD cmd, EE_VA va, void* val, uint16_t size);
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
    EE_AesKey(EE_CMD_R, EE_NULL);
    EE_Mode(EE_CMD_R, EE_NULL);
    for (uint8_t mTrip = 0; mTrip < HBAR_M_TRIP_MAX; mTrip++)
      EE_TripMeter(EE_CMD_R, mTrip, EE_NULL);
    for (uint8_t m = 0; m < HBAR_M_MAX; m++)
    	EE_SubMode(EE_CMD_R, m, EE_NULL);

    EE_FotaVersion(EE_CMD_R, EE_NULL);
#endif
    EE_FotaType(EE_CMD_R, EE_NULL);
  } else
    printf("EEPROM:Error\n");
  unlock();

  return ok;
}

#if (APP)
uint8_t EE_AesKey(EE_CMD cmd, uint32_t val[4]) {
  uint32_t *ptr = val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = AES_KEY;
  ok = Cmd(cmd, VA_AES_KEY, ptr, sizeof(AES_KEY));
  if (cmd == EE_CMD_W)
  	memcpy(AES_KEY, val, sizeof(AES_KEY));

  return ok;
}

uint8_t EE_TripMeter(EE_CMD cmd, HBAR_MODE_TRIP mTrip, uint16_t val) {
	uint16_t *ptr = &val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = &HBAR.d.trip[mTrip];
  ok = Cmd(cmd, VA_TRIP_A + mTrip, ptr, sizeof(val));
  if (cmd == EE_CMD_W)
  	memcpy(&(HBAR.d.trip[mTrip]), &val, sizeof(val));

  if (mTrip == HBAR_M_TRIP_ODO)
  	HBAR.d.meter = val * 1000;

  return ok;
}

uint8_t EE_SubMode(EE_CMD cmd, HBAR_MODE m, uint8_t val) {
	uint8_t *ptr = &val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = &HBAR.d.mode[m];
  ok = Cmd(cmd, VA_MODE_DRIVE + m, ptr, sizeof(val));
  if (cmd == EE_CMD_W)
  	memcpy(&(HBAR.d.mode[m]), &val, sizeof(val));

  if (HBAR.d.mode[m] > HBAR_SubModeMax(m))
  	HBAR.d.mode[m] = 0;

  return ok;
}

uint8_t EE_Mode(EE_CMD cmd, uint8_t val) {
	uint8_t *ptr = &val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = &HBAR.d.m;
  ok = Cmd(cmd, VA_MODE, ptr, sizeof(val));
  if (cmd == EE_CMD_W)
  	memcpy(&HBAR.d.m, &val, sizeof(val));

  if (HBAR.d.m >= HBAR_M_MAX)
  	HBAR.d.m = 0;

  return ok;
}

#endif

uint8_t EE_FotaType(EE_CMD cmd, IAP_TYPE val) {
	IAP_TYPE *ptr = &val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = &FOTA.TYPE;
  ok = Cmd(cmd, VA_FOTA_TYPE, ptr, sizeof(val));
  if (cmd == EE_CMD_W)
  	memcpy(&FOTA.TYPE, &val, sizeof(val));

  if (!(FOTA.TYPE == IAP_VCU || FOTA.TYPE == IAP_HMI))
  	FOTA.TYPE = IAP_VCU;

  return ok;
}

uint8_t EE_FotaFlag(EE_CMD cmd, uint32_t val) {
	uint32_t *ptr = &val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = &FOTA.FLAG;
  ok = Cmd(cmd, VA_FOTA_FLAG, ptr, sizeof(val));
  if (cmd == EE_CMD_W)
  	memcpy(&FOTA.FLAG, &val, sizeof(val));

  return ok;
}

uint8_t EE_FotaVersion(EE_CMD cmd, uint16_t val) {
	uint16_t *ptr = &val;
  uint8_t ok;

  if (cmd == EE_CMD_R)
  	ptr = &FOTA.VERSION;
  ok = Cmd(cmd, VA_FOTA_VERSION, ptr, sizeof(val));
  if (cmd == EE_CMD_W)
  	memcpy(&FOTA.VERSION, &val, sizeof(val));

  return ok;
}

//uint8_t EE_NetCon(EE_CMD cmd, net_con_t con) {
//  net_con_t* d = &SIM.net.con;
//  uint8_t ok = 0;
//
//  ok += Cmd(VA_NET_CON_APN, cmd, con.apn, d->apn, sizeof(d->apn));
//  ok += Cmd(VA_NET_CON_USER, cmd, con.user, d->user, sizeof(d->user));
//  ok += Cmd(VA_NET_CON_PASS, cmd, con.pass, d->pass, sizeof(d->pass));
//
//  return ok == 3;
//}
//
//uint8_t EE_NetFtp(EE_CMD cmd, net_ftp_t ftp) {
//  net_ftp_t* d = &SIM.net.ftp;
//  uint8_t ok = 0;
//
//  ok += Cmd(VA_NET_FTP_HOST, cmd, ftp.host, d->host, sizeof(d->host));
//  ok += Cmd(VA_NET_FTP_USER, cmd, ftp.user, d->user, sizeof(d->user));
//  ok += Cmd(VA_NET_FTP_PASS, cmd, ftp.pass, d->pass, sizeof(d->pass));
//
//  return ok == 3;
//}
//
//uint8_t EE_NetMqtt(EE_CMD cmd, net_mqtt_t mqtt) {
//  net_mqtt_t* d = &SIM.net.mqtt;
//  uint8_t ok = 0;
//
//  ok += Cmd(VA_NET_MQTT_HOST, cmd, mqtt.host, d->host, sizeof(d->host));
//  ok += Cmd(VA_NET_MQTT_PORT, cmd, &(mqtt.port), &(d->port), sizeof(d->port));
//  ok += Cmd(VA_NET_MQTT_USER, cmd, mqtt.user, d->user, sizeof(d->user));
//  ok += Cmd(VA_NET_MQTT_PASS, cmd, mqtt.pass, d->pass, sizeof(d->pass));
//
//  return ok == 4;
//}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t Cmd(EE_CMD cmd, EE_VA va, void* val, uint16_t size) {
  uint16_t addr = EE_WORD(va);
  uint8_t ok = 0;

  lock();
  if (cmd == EE_CMD_W)
    ok = EEPROM24XX_Save(addr, val, size);
  else
    ok = EEPROM24XX_Load(addr, val, size);
  unlock();

  return ok;
}

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
