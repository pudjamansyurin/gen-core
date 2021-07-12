/*
 * mqtt.h
 *
 *  Created on: Jan 17, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__MQTT_H_
#define INC_LIBS__MQTT_H_

/* Includes
 * --------------------------------------------*/
#include "App/command.h"
#include "App/reporter.h"

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
