/*
 * _mqtt.c
 *
 *  Created on: Jan 17, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "MQTTPacket.h"
#include "Drivers/_simcom.h"
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"

/* Private variables ----------------------------------------------------------*/
static unsigned char buf[256];
static int buflen = sizeof(buf);

/* Private functions prototype ------------------------------------------------*/
static uint8_t MQTT_Upload(unsigned char *buf, uint16_t len, uint8_t reply);

/* Public functions implementation -------------------------------------------*/
uint8_t MQTT_Connect(void) {
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char sessionPresent,	connack_rc;
  char clientID[9];
	int len;

	sprintf(clientID, "%08X", (unsigned int) VCU.d.unit_id);
	data.clientID.cstring = clientID;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	data.username.cstring = "garda";
	data.password.cstring = "energi";

	len = MQTTSerialize_connect(buf, buflen, &data);

	if (!MQTT_Upload(buf, len, CONNACK))
		return 0;

	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0) {
		printf("MQTT:Unable to connect, RC %d\n", connack_rc);
		return 0;
	}

	printf("MQTT:Connection established\n");
	return 1;
}

uint8_t MQTT_Subscribe(char *topic) {
	MQTTString topicFilters = MQTTString_initializer;
	unsigned char dup = 0;
	unsigned short packetid = 1;
	int qoss = 0, count = 1, len;

	topicFilters.cstring = topic;
	len = MQTTSerialize_subscribe(
			buf, buflen,
			dup, packetid, count,
			&topicFilters, &qoss
	);

	if (!MQTT_Upload(buf, len, SUBACK))
		return 0;

	MQTTDeserialize_suback(&packetid, 1, &count, &qoss, buf, buflen);
	if (qoss != 0) {
		printf("MQTT:Granted QoS != 0, %d\n", qoss);
		return 0;
	}

	printf("MQTT:Subscribed successfully.");
	return 1;
}

uint8_t MQTT_Publish(char *topic, void *payload, uint16_t payloadlen) {
	MQTTString topicName = MQTTString_initializer;
	unsigned char dup = 0, retained = 0;
	unsigned short packetid = 0;
	int qos = 0, len;

	topicName.cstring = topic;
	len = MQTTSerialize_publish(
			buf, buflen,
			dup, qos, retained, packetid,
			topicName, payload, payloadlen
	);

	if (!MQTT_Upload(buf, len, 0))
		return 0;

	printf("MQTT:Published successfully.");
	return 1;
}

uint8_t MQTT_Disconnect(void) {
	int len;

	len = MQTTSerialize_disconnect(buf, buflen);

	if (!MQTT_Upload(buf, len, 0))
		return 0;

	printf("MQTT:Disconnect successfully.");
	return 1;
}

uint16_t MQTT_Receive(void) {
	MQTTString topicName;
	unsigned char dup, retained, *dest;
	unsigned short packetid;
	int qos, len = 0;

	if (MQTTPacket_read(buf, buflen, Simcom_GetData) != PUBLISH)
		return 0;

	MQTTDeserialize_publish(
			&dup, &qos, &retained, &packetid,
			&topicName, &dest, &len,
			buf, buflen
	);

	printf("MQTT:Receive = %.*s\n", len, dest);
	return len;
}

/* Private functions implementation -------------------------------------------*/
static uint8_t MQTT_Upload(unsigned char *buf, uint16_t len, uint8_t reply) {
	if (Simcom_Upload(buf, len) != SIM_RESULT_OK)
		return 0;

	if (reply) {
		if (!Simcom_GetServerResponse(5000))
			return 0;

		if (MQTTPacket_read(buf, buflen, Simcom_GetData) != reply)
			return 0;
	}

	return 1;
}
