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
static uint32_t last_con = 0;

/* Private functions prototype ------------------------------------------------*/
static uint8_t MQTT_Subscribe(char *topic);
static uint8_t MQTT_Publish(char *topic, void *payload, uint16_t payloadlen);
static uint8_t MQTT_Upload(unsigned char *buf, uint16_t len, uint8_t reply,  uint16_t timeout);

/* Public functions implementation -------------------------------------------*/
uint8_t MQTT_DoPublish(payload_t *payload) {
	char topic[20];

	sprintf(topic, "VCU/%lu/%3s",
			VCU.d.unit_id,
			(payload->type == PAYLOAD_REPORT ? "RPT" : "RSP")
	);

	return MQTT_Publish(topic, payload->pPayload, payload->size);
}

uint8_t MQTT_DoSubscribe(void) {
	char topic[20];

	sprintf(topic, "VCU/%lu/CMD", VCU.d.unit_id);

	return MQTT_Subscribe(topic);
}

uint8_t MQTT_Connect(void) {
	unsigned char buf[256];
	int buflen = sizeof(buf);
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char sessionPresent,	connack_rc;
	int len;

	data.keepAliveInterval = MQTT_KEEPALIVE;
	data.cleansession = 1;
	data.username.cstring = MQTT_USERNAME;
	data.password.cstring = MQTT_PASSWORD;

	len = MQTTSerialize_connect(buf, buflen, &data);

	if (!MQTT_Upload(buf, len, CONNACK, 10000))
		return 0;

	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0) {
		printf("MQTT:Unable to connect, RC %d\n", connack_rc);
		return 0;
	}

	printf("MQTT:Connected\n");
	last_con = _GetTickMS();
	return 1;
}

uint8_t MQTT_Disconnect(void) {
	unsigned char buf[2];
	int buflen = sizeof(buf);
	int len = MQTTSerialize_disconnect(buf, buflen);

	if (!MQTT_Upload(buf, len, 0, 0))
		return 0;

	printf("MQTT:Disconnected\n");
	last_con = _GetTickMS();
	return 1;
}

uint8_t MQTT_Ping(void) {
	unsigned char buf[2];
	int buflen = sizeof(buf);
	int len;

	if ((_GetTickMS() - last_con) < (MQTT_KEEPALIVE * 1000) / 2)
		return 1;

	len = MQTTSerialize_pingreq(buf, buflen);

	if (!MQTT_Upload(buf, len, PINGRESP, 5000))
		return 0;

	printf("MQTT:Pinged\n");
	last_con = _GetTickMS();
	return 1;
}

uint8_t MQTT_Receive(command_t *cmd, void *buffer) {
	unsigned char buf[100];
	int buflen = sizeof(buf);
	MQTTString topicName;
	unsigned char dup, retained, *dst;
	unsigned short packetid;
	int qos, len = 0;
	uint32_t ptr = (uint32_t) buffer;

	if (MQTTPacket_read(buf, buflen, Simcom_GetData) != PUBLISH) {
		buffer = (void*) ptr;
		return 0;
	}

	MQTTDeserialize_publish(
			&dup, &qos, &retained, &packetid,
			&topicName, &dst, &len,
			buf, buflen
	);

	memcpy(cmd, dst, len);

	printf("MQTT:Received %d bytes\n", len);
	return len;
}

/* Private functions implementation -------------------------------------------*/
static uint8_t MQTT_Subscribe(char *topic) {
	unsigned char buf[256];
	int buflen = sizeof(buf);
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

	if (!MQTT_Upload(buf, len, SUBACK, 5000))
		return 0;

	MQTTDeserialize_suback(&packetid, 1, &count, &qoss, buf, buflen);
	if (qoss != 0) {
		printf("MQTT:Granted QoS != 0, %d\n", qoss);
		return 0;
	}

	printf("MQTT:Subscribed\n");
	last_con = _GetTickMS();
	return 1;
}

static uint8_t MQTT_Publish(char *topic, void *payload, uint16_t payloadlen) {
	unsigned char buf[256];
	int buflen = sizeof(buf);
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

	if (!MQTT_Upload(buf, len, 0, 0))
		return 0;

	printf("MQTT:Published\n");
	last_con = _GetTickMS();
	return 1;
}

static uint8_t MQTT_Upload(unsigned char *buf, uint16_t len, uint8_t reply, uint16_t timeout) {
	if (Simcom_Upload(buf, len) != SIM_RESULT_OK)
		return 0;

	if (reply) {
		if (!Simcom_GetServerResponse(timeout))
			return 0;

		if (MQTTPacket_read(buf, len, Simcom_GetData) != reply)
			return 0;
	}

	return 1;
}
