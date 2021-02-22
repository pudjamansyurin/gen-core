/*
 * _mqtt.h
 *
 *  Created on: Jan 17, 2021
 *      Author: pudja
 */

#ifndef INC_LIBS__MQTT_H_
#define INC_LIBS__MQTT_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Libs/_command.h"

/* Exported constants --------------------------------------------------------*/
#define MQTT_USERNAME          						""
#define MQTT_PASSWORD          						""
#define MQTT_KEEPALIVE          		  (uint8_t) 60				// in second

/* Public functions prototypes ----------------------------------------------*/
uint8_t MQTT_Publish(payload_t *payload);
uint8_t MQTT_Subscribe(void);
uint8_t MQTT_Unsubscribe(void);
uint8_t MQTT_Connect(void);
uint8_t MQTT_Disconnect(void);
uint8_t MQTT_Ping(void);
uint8_t MQTT_Received(void);
uint8_t MQTT_Receive(command_t *cmd);

#endif /* INC_LIBS__MQTT_H_ */
