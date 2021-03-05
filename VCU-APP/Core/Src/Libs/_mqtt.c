/*
 * _mqtt.c
 *
 *  Created on: Jan 17, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_simcom.h"
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"

/* Private variables ----------------------------------------------------------*/
static mqtt_t MQTT = {
		.tick = 0,
		.packetid = 0,
		.qos = {
				.command = 2,
				.response = 1,
				.report = 1,
				.will = 1
		},
		.rx = {
				.received = 0
		}
};

/* Private functions prototype ------------------------------------------------*/
static uint8_t Subscribe(char *topic, int qos);
static uint8_t Unsubscribe(char *topic);
static uint8_t Publish(void *payload, uint16_t payloadlen, char *topic, int qos);
static uint8_t Upload(unsigned char *buf, uint16_t len, uint8_t reply,  uint16_t timeout);

/* Public functions implementation -------------------------------------------*/
uint8_t MQTT_Publish(payload_t *payload) {
	char *topic;
	int qos;

	if (payload->type == PAYLOAD_REPORT) {
		qos = MQTT.qos.report;
		topic = MQTT.topic.report;
	} else {
		qos = MQTT.qos.response;
		topic = MQTT.topic.response;
	}

	return Publish(payload->pPayload, payload->size, topic, qos);
}

uint8_t MQTT_PublishWill(uint8_t on) {
	char status[2];

	sprintf(status, "%1d", on);
	return Publish(status, strlen(status), MQTT.topic.will, MQTT.qos.will);
}

uint8_t MQTT_Subscribe(void) {
	return Subscribe(MQTT.topic.command, MQTT.qos.command);
}

uint8_t MQTT_Unsubscribe(void) {
	return Unsubscribe(MQTT.topic.command);
}

uint8_t MQTT_Connect(void) {
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char sessionPresent, connack_rc;
	char clientId[20], status[] = "0";
	unsigned char buf[256];
	int len, buflen = sizeof(buf);

	// generate topics
	sprintf(MQTT.topic.command, "VCU/%lu/CMD", VCU.d.unit_id);
	sprintf(MQTT.topic.response, "VCU/%lu/RSP", VCU.d.unit_id);
	sprintf(MQTT.topic.report, "VCU/%lu/RPT", VCU.d.unit_id);
	sprintf(MQTT.topic.will, "VCU/%lu/STS", VCU.d.unit_id);
	sprintf(clientId, "VCU-%lu", VCU.d.unit_id);

	// subscribe
	data.cleansession = 0;
	data.clientID.cstring = clientId;
	data.keepAliveInterval = MQTT_KEEPALIVE;
	data.username.cstring = MQTT_USERNAME;
	data.password.cstring = MQTT_PASSWORD;

	data.willFlag = 1;
	data.will.retained = 1;
	data.will.qos = MQTT.qos.will;
	data.will.topicName.cstring = MQTT.topic.will;
	data.will.message.cstring = status;

	len = MQTTSerialize_connect(buf, buflen, &data);

	if (!Upload(buf, len, CONNACK, 5000))
		return 0;

	if (!MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) || connack_rc != 0) {
		printf("MQTT:Connect failed, RC %d\n", connack_rc);
		return 0;
	}

	printf("MQTT:Connected\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

uint8_t MQTT_Disconnect(void) {
	unsigned char buf[2];
	int len, buflen = sizeof(buf);

	len = MQTTSerialize_disconnect(buf, buflen);

	if (!Upload(buf, len, 0, 0))
		return 0;

	printf("MQTT:Disconnected\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

uint8_t MQTT_Ping(void) {
	unsigned char buf[2];
	int len, buflen = sizeof(buf);

	if ((_GetTickMS() - MQTT.tick) <= (MQTT_KEEPALIVE *1000))
		return 1;

	len = MQTTSerialize_pingreq(buf, buflen);

	if (!Upload(buf, len, PINGRESP, 5000))
		return 0;

	printf("MQTT:Pinged\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

uint8_t MQTT_GotPublish(void) {
	unsigned char buf[256], *dst;
	int len, buflen = sizeof(buf);
	mqtt_data_t *d = &(MQTT.rx.d);

	if (MQTT.rx.received)	return 0;

	d->packettype = MQTTPacket_read(buf, buflen, Simcom_GetData);
	if (d->packettype != PUBLISH)
		return 0;

	if (!MQTTDeserialize_publish(
			&(d->dup), &(d->qos), &(d->retained), &(d->packetid),
			&(d->topicName), &dst, &len,
			buf, buflen
	))
		return 0;

	if (!CMD_ValidateCommand(dst, len)) return 0;
	memcpy(&(MQTT.rx.command), dst, len);

	MQTT.rx.received = 1;
	return 1;
}

uint8_t MQTT_GotCommand(void) {
	return MQTT.rx.received;
}

uint8_t MQTT_AckPublish(command_rx_t *cmd) {
	unsigned char buf[100], packettype;
	int len, buflen = sizeof(buf);
	mqtt_data_t *d = &(MQTT.rx.d);
	unsigned short packetid;

	MQTT.rx.received = 0;
	memcpy(cmd, &(MQTT.rx.command), sizeof(command_rx_t));

	if (d->qos == 1) {
		len = MQTTSerialize_puback(buf, buflen, d->packetid);
		if (!Upload(buf, len, 0, 5000))
			return 0;
	}

	else if (d->qos == 2) {
		len = MQTTSerialize_ack(buf, buflen, PUBREC, d->dup, d->packetid);
		if (!Upload(buf, len, PUBREL, 5000))
			return 0;

		if (!MQTTDeserialize_ack(&packettype, &(d->dup), &packetid, buf, buflen))
			return 0;

		if (d->packetid != packetid) {
			printf("MQTT:PUBREL packet mismatched\n");
			return 0;
		}

		len = MQTTSerialize_pubcomp(buf, buflen, packetid);
		if (!Upload(buf, len, 0, 5000))
			return 0;
	}

	printf("MQTT:Received %d bytes\n", len);
	return len;
}

/* Private functions implementation -------------------------------------------*/
static uint8_t Subscribe(char *topic, int qos) {
	MQTTString topicFilters[] = { MQTTString_initializer };
	int count = 1, count_rx;
	int grantedQos, qoss_rx[count], qoss[] = { qos };
	unsigned char buf[256];
	int len, buflen = sizeof(buf);
	mqtt_data_t d;

	topicFilters[0].cstring = topic;
	len = MQTTSerialize_subscribe(
			buf, buflen,
			0, ++MQTT.packetid, count,
			topicFilters, qoss
	);

	if (!Upload(buf, len, SUBACK, 5000))
		return 0;

	if (!MQTTDeserialize_suback(&(d.packetid), count, &count_rx, qoss_rx, buf, buflen)) {
		printf("MQTT:Subscribe failed\n");
		return 0;
	}

	if (MQTT.packetid != d.packetid) {
		printf("MQTT:SUBACK packet mismatched\n");
		return 0;
	}

	grantedQos = qoss_rx[0];
	if (qos != grantedQos) {
		printf("MQTT:Granted QoS != %d, %d\n", qos, grantedQos);
		// return 0;
	}

	printf("MQTT:Subscribed\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

static uint8_t Unsubscribe(char *topic) {
	MQTTString topicFilters = MQTTString_initializer;
	unsigned char buf[256];
	int len, buflen = sizeof(buf);
	int count = 1;
	mqtt_data_t d;

	topicFilters.cstring = topic;
	len = MQTTSerialize_unsubscribe(buf, buflen, 0, ++MQTT.packetid, count, &topicFilters);

	if (!Upload(buf, len, UNSUBACK, 5000))
		return 0;

	if (!MQTTDeserialize_unsuback(&(d.packetid), buf, buflen)) {
		printf("MQTT:Unsubscribe failed\n");
		return 0;
	}

	if (MQTT.packetid != d.packetid) {
		printf("MQTT:UNSUBACK packet mismatched\n");
		return 0;
	}

	printf("MQTT:Unsubscribed\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

static uint8_t Publish(void *payload, uint16_t payloadlen, char *topic, int qos) {
	MQTTString topicName = MQTTString_initializer;
	unsigned char buf[256];
	int len, buflen = sizeof(buf);
	mqtt_data_t d;
	uint8_t reply = 0;

	topicName.cstring = topic;
	len = MQTTSerialize_publish(
			buf, buflen,
			0, qos, 1, ++MQTT.packetid,
			topicName, payload, payloadlen
	);

	if (qos)
		reply = (qos == 1) ? PUBACK : PUBREC;

	if (!Upload(buf, len, reply, 5000))
		return 0;

	if (qos == 1) {
		if (!MQTTDeserialize_ack(&(d.packettype), &(d.dup), &(d.packetid), buf, buflen))
			return 0;

		if (MQTT.packetid != d.packetid) {
			printf("MQTT:PUBACK packet mismatched\n");
			return 0;
		}
	}

	else if (qos == 2) {
		if (!MQTTDeserialize_ack(&(d.packettype), &(d.dup), &(d.packetid), buf, buflen))
			return 0;

		if (MQTT.packetid != d.packetid) {
			printf("MQTT:PUBREC packet mismatched\n");
			return 0;
		}

		len = MQTTSerialize_pubrel(buf, buflen, d.dup, MQTT.packetid);
		if (!Upload(buf, len, PUBCOMP, 5000))
			return 0;

		if (!MQTTDeserialize_ack(&(d.packettype), &(d.dup), &(d.packetid), buf, buflen))
			return 0;

		if (MQTT.packetid != d.packetid) {
			printf("MQTT:PUBCOMP packet mismatched\n");
			return 0;
		}
	}

	printf("MQTT:Published\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

static uint8_t Upload(unsigned char *buf, uint16_t len, uint8_t reply, uint16_t timeout) {
	if (!Simcom_Upload(buf, len))
		return 0;

	if (reply) {
		if (!Simcom_ReceivedResponse(timeout))
			return 0;

		if (MQTTPacket_read(buf, len, Simcom_GetData) != reply)
			return 0;
	}

	return 1;
}
