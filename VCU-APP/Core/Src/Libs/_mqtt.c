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
static int grantedQos;
static unsigned short packetid = 0;
static char cmdTopic[20], rptTopic[20], rspTopic[20];
static unsigned char buf_rx[100];
static int buflen_rx = sizeof(buf_rx);
static uint8_t received = 0;

/* Private functions prototype ------------------------------------------------*/
static uint8_t MQTT_Subscribe(char *topic, int qos);
static uint8_t MQTT_Unsubscribe(char *topic);
static uint8_t MQTT_Publish(void *payload, uint16_t payloadlen, char *topic, int qos);
static uint8_t MQTT_Upload(unsigned char *buf, uint16_t len, uint8_t reply,  uint16_t timeout);

/* Public functions implementation -------------------------------------------*/
uint8_t MQTT_DoPublish(payload_t *payload) {
	char *topic = (payload->type == PAYLOAD_REPORT ? rptTopic : rspTopic);
	int qos = (payload->type == PAYLOAD_REPORT ? 1 : 2);

	return MQTT_Publish(payload->pPayload, payload->size, topic, qos);
}

uint8_t MQTT_DoSubscribe(void) {
	return MQTT_Subscribe(cmdTopic, 2);
}

uint8_t MQTT_DoUnsubscribe(void) {
	return MQTT_Unsubscribe(cmdTopic);
}

uint8_t MQTT_Connect(void) {
	unsigned char buf[256];
	char clientId[20];
	int buflen = sizeof(buf);
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char sessionPresent,	connack_rc;
	int len;

	// generate topics
	sprintf(cmdTopic, "VCU/%lu/CMD", VCU.d.unit_id);
	sprintf(rptTopic, "VCU/%lu/RPT", VCU.d.unit_id);
	sprintf(rspTopic, "VCU/%lu/RSP", VCU.d.unit_id);
	sprintf(clientId, "VCU-%lu", VCU.d.unit_id);

	// subscribe
	data.clientID.cstring = clientId;
	data.keepAliveInterval = MQTT_KEEPALIVE;
	data.cleansession = 1;
	data.username.cstring = MQTT_USERNAME;
	data.password.cstring = MQTT_PASSWORD;

	len = MQTTSerialize_connect(buf, buflen, &data);

	if (!MQTT_Upload(buf, len, CONNACK, 5000))
		return 0;

	if (!MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) || connack_rc != 0) {
		printf("MQTT:Connect failed, RC %d\n", connack_rc);
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

	if ((_GetTickMS() - last_con) <= (MQTT_KEEPALIVE *1000))
		return 1;

	len = MQTTSerialize_pingreq(buf, buflen);

	if (!MQTT_Upload(buf, len, PINGRESP, 5000))
		return 0;

	printf("MQTT:Pinged\n");
	last_con = _GetTickMS();
	return 1;
}

uint8_t MQTT_Received(void) {
	if (received) return 0;

	received = MQTTPacket_read(buf_rx, buflen_rx, Simcom_GetData) == PUBLISH;
	return received;
}

uint8_t MQTT_Receive(command_t *cmd) {
	unsigned char buf[100];
	int buflen = sizeof(buf);
	MQTTString topicName;
	unsigned char dup, retained, packettype, *dst;
	unsigned short packetid_rx, packetid_rx2;
	int qos, len = 0;

	if (!received) return 0;
	received = 0;

	if (!MQTTDeserialize_publish(
			&dup, &qos, &retained, &packetid_rx,
			&topicName, &dst, &len,
			buf_rx, buflen_rx
	))
		return 0;

	memcpy(cmd, dst, len);

	if (grantedQos == 1) {
		len = MQTTSerialize_puback(buf, buflen, packetid_rx);
		if (!MQTT_Upload(buf, len, 0, 5000))
			return 0;
	}

	else if (grantedQos == 2) {
		len = MQTTSerialize_ack(buf, buflen, PUBREC, dup, packetid_rx);
		if (!MQTT_Upload(buf, len, PUBREL, 5000))
			return 0;

		if (!MQTTDeserialize_ack(&packettype, &dup, &packetid_rx2, buf, buflen))
			return 0;

		if (packetid_rx != packetid_rx2) {
			printf("MQTT:PUBREL packet mismatched\n");
			return 0;
		}

		len = MQTTSerialize_pubcomp(buf, buflen, packetid_rx2);
		if (!MQTT_Upload(buf, len, 0, 5000))
			return 0;
	}

	printf("MQTT:Received %d bytes\n", len);
	return len;
}

/* Private functions implementation -------------------------------------------*/
static uint8_t MQTT_Subscribe(char *topic, int qos) {
	unsigned char buf[256];
	int buflen = sizeof(buf);
	int count = 1, count_rx, len;
	MQTTString topicFilters[] = { MQTTString_initializer };
	int qoss_rx[count], qoss[] = { qos };
	unsigned char dup = 0;
	unsigned short packetid_rx;

	packetid++;

	topicFilters[0].cstring = topic;
	len = MQTTSerialize_subscribe(
			buf, buflen,
			dup, packetid, count,
			topicFilters, qoss
	);

	if (!MQTT_Upload(buf, len, SUBACK, 5000))
		return 0;

	if (!MQTTDeserialize_suback(&packetid_rx, count, &count_rx, qoss_rx, buf, buflen)) {
		printf("MQTT:Subscribe failed\n");
		return 0;
	}

	if (packetid != packetid_rx) {
		printf("MQTT:SUBACK packet mismatched\n");
		return 0;
	}

	grantedQos = qoss_rx[0];
	if (qos != grantedQos) {
		printf("MQTT:Granted QoS != %d, %d\n", qos, grantedQos);
		// return 0;
	}

	printf("MQTT:Subscribed\n");
	last_con = _GetTickMS();
	return 1;
}

static uint8_t MQTT_Unsubscribe(char *topic) {
	unsigned char buf[256];
	int buflen = sizeof(buf);
	MQTTString topicFilters = MQTTString_initializer;
	unsigned char dup = 0;
	unsigned short packetid_rx;
	int count = 1, len;

	packetid++;

	topicFilters.cstring = topic;
	len = MQTTSerialize_unsubscribe(buf, buflen, dup, packetid, count, &topicFilters);

	if (!MQTT_Upload(buf, len, UNSUBACK, 5000))
		return 0;

	if (!MQTTDeserialize_unsuback(&packetid_rx, buf, buflen)) {
		printf("MQTT:Unsubscribe failed\n");
		return 0;
	}

	if (packetid != packetid_rx) {
		printf("MQTT:UNSUBACK packet mismatched\n");
		return 0;
	}

	printf("MQTT:Unsubscribed\n");
	last_con = _GetTickMS();
	return 1;
}

static uint8_t MQTT_Publish(void *payload, uint16_t payloadlen, char *topic, int qos) {
	unsigned char buf[256];
	int buflen = sizeof(buf);
	MQTTString topicName = MQTTString_initializer;
	unsigned char dup = 0, retained = 1, packettype;
	unsigned short packetid_rx;
	int len;
	uint8_t reply = 0;

	packetid++;

	topicName.cstring = topic;
	len = MQTTSerialize_publish(
			buf, buflen,
			dup, qos, retained, packetid,
			topicName, payload, payloadlen
	);

	if (qos)
		reply = (qos == 1) ? PUBACK : PUBREC;

	if (!MQTT_Upload(buf, len, reply, 5000))
		return 0;

	if (qos == 1) {
		if (!MQTTDeserialize_ack(&packettype, &dup, &packetid_rx, buf, buflen))
			return 0;

		if (packetid != packetid_rx) {
			printf("MQTT:PUBACK packet mismatched\n");
			return 0;
		}
	}

	else if (qos == 2) {
		if (!MQTTDeserialize_ack(&packettype, &dup, &packetid_rx, buf, buflen))
			return 0;

		if (packetid != packetid_rx) {
			printf("MQTT:PUBREC packet mismatched\n");
			return 0;
		}

		len = MQTTSerialize_pubrel(buf, buflen, dup, packetid);
		if (!MQTT_Upload(buf, len, PUBCOMP, 5000))
			return 0;

		if (!MQTTDeserialize_ack(&packettype, &dup, &packetid_rx, buf, buflen))
			return 0;

		if (packetid != packetid_rx) {
			printf("MQTT:PUBCOMP packet mismatched\n");
			return 0;
		}
	}

	printf("MQTT:Published\n");
	last_con = _GetTickMS();
	return 1;
}

static uint8_t MQTT_Upload(unsigned char *buf, uint16_t len, uint8_t reply, uint16_t timeout) {
	if (Simcom_Upload(buf, len) != SIM_RESULT_OK)
		return 0;

	if (reply) {
		if (!Simcom_ReceiveResponse(timeout))
			return 0;

		if (MQTTPacket_read(buf, len, Simcom_GetData) != reply)
			return 0;
	}

	return 1;
}
