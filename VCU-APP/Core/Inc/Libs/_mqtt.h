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

/* Public functions prototypes ----------------------------------------------*/
uint8_t MQTT_Connect(void);
uint8_t MQTT_Subscribe(char *topic);
uint8_t MQTT_Publish(char *topic, void *payload, uint16_t payloadlen);
uint8_t MQTT_Disconnect(void);
uint16_t MQTT_Receive(void);

#endif /* INC_LIBS__MQTT_H_ */
