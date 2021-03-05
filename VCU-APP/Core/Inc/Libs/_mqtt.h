/*
 * _mqtt.h
 *
 *  Created on: Jan 17, 2021
 *      Author: pudja
 */

#ifndef INC_LIBS__MQTT_H_
#define INC_LIBS__MQTT_H_

/* Includes ------------------------------------------------------------------*/
#include "MQTTPacket.h"
#include "Libs/_utils.h"
#include "Libs/_command.h"

/* Exported constants --------------------------------------------------------*/
#define MQTT_USERNAME          						""
#define MQTT_PASSWORD          						""
#define MQTT_KEEPALIVE          (uint8_t) 5				// in second

/* Exported structs --------------------------------------------------------*/
typedef struct {
	MQTTString topicName;
	unsigned short packetid;
	unsigned char dup;
	unsigned char retained;
	unsigned char packettype;
	int qos;
} mqtt_data_t;

typedef struct {
	unsigned short packetid;
	uint32_t tick;
	struct {
		int command;
		int response;
		int report;
		int will;
	} qos;
	struct {
		char command[20];
		char response[20];
		char report[20];
		char will[20];
	} topic;
	struct {
		command_rx_t command;
		uint8_t received;
		mqtt_data_t d;
	} rx;
} mqtt_t;

/* Public functions prototypes ----------------------------------------------*/
uint8_t MQTT_Publish(payload_t *payload);
uint8_t MQTT_PublishWill(uint8_t on);
uint8_t MQTT_Subscribe(void);
uint8_t MQTT_Unsubscribe(void);
uint8_t MQTT_Connect(void);
uint8_t MQTT_Disconnect(void);
uint8_t MQTT_Ping(void);
uint8_t MQTT_GotPublish(void);
uint8_t MQTT_GotCommand(void);
uint8_t MQTT_AckPublish(command_rx_t *cmd);

#endif /* INC_LIBS__MQTT_H_ */
