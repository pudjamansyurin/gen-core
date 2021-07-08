/*
 * _mqtt.h
 *
 *  Created on: Jan 17, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__MQTT_H_
#define INC_LIBS__MQTT_H_

/* Includes
 * --------------------------------------------*/
#include "App/_command.h"
#include "App/_reporter.h"

/* Exported constants
 * --------------------------------------------*/
#define MQTT_PERSIST_SESSION 0
#define MQTT_KEEPALIVE_S ((uint8_t)30)
#define MQTT_UPLOAD_MS ((uint16_t)10000)

/* Exported types
 * --------------------------------------------*/
typedef struct {
  char topic[20];
  unsigned short packetid;
  unsigned char dup;
  unsigned char retained;
  unsigned char packettype;
  int qos;
} mqtt_handler_t;

typedef struct {
  uint8_t pending;
  command_t command;
  mqtt_handler_t d;
} mqtt_rx_t;

typedef struct {
  unsigned short packetid;
  uint8_t subscribed;
  uint8_t willed;
  uint32_t tick;
  struct {
    int will;
    int command;
    int response;
    struct {
      int simple;
      int full;
    } report;
  } qos;
  struct {
    char command[20];
    char response[20];
    char report[20];
    char will[20];
  } topic;
  mqtt_rx_t rx;
} mqtt_t;

/* Public functions prototypes
 * --------------------------------------------*/
uint8_t MQTT_Connect(void);
uint8_t MQTT_Disconnect(void);
uint8_t MQTT_Ping(void);

uint8_t MQTT_Subscribe(void);
uint8_t MQTT_Unsubscribe(void);

uint8_t MQTT_Publish(const payload_t *payload);
uint8_t MQTT_PublishWill(uint8_t on);

uint8_t MQTT_GotPublish(void);
uint8_t MQTT_AckPublish(command_t *cmd);
uint8_t MQTT_GotCommand(void);
void MQTT_FlushCommand(void);

uint8_t MQTT_Subscribed(void);
uint8_t MQTT_Willed(void);

#endif /* INC_LIBS__MQTT_H_ */
