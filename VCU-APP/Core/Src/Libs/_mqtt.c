/*
 * _mqtt.c
 *
 *  Created on: Jan 17, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_mqtt.h"
#include "Drivers/_simcom.h"
#include "MQTTPacket.h"

/* Private variables
 * ----------------------------------------------------------*/
static mqtt_t MQTT = {
		.tick = 0,
		.packetid = 0,
		.willed = 0,
		.subscribed = 0,
		.qos = {
				.will = 1,
				.command = 0,
				.response = 1,
				.report = {
						.simple = 0,
						.full = 0,
				},
		},
		.rx = {
				.pending = 0,
		}
};

/* Private functions prototype
 * ------------------------------------------------*/
static uint8_t Publish(void *payload, uint16_t payloadlen, char *topic, int qos, uint8_t retained);
static uint8_t Upload(unsigned char *buf, uint16_t len, uint8_t reply, uint16_t timeout);

/* Public functions implementation -------------------------------------------*/
uint8_t MQTT_Publish(payload_t *payload) {
	uint8_t retained = 1;
	char *topic;
	int qos;

	if (payload->type == PAYLOAD_REPORT) {
		uint8_t fullFrame = payload->size == sizeof(report_t);

		qos = fullFrame ? MQTT.qos.report.full : MQTT.qos.report.simple;
		topic = MQTT.topic.report;
		retained = fullFrame;
	} else {
		qos = MQTT.qos.response;
		topic = MQTT.topic.response;
	}

	return Publish(payload->data, payload->size, topic, qos, retained);
}

uint8_t MQTT_PublishWill(uint8_t on) {
	char status[2];

	MQTT.willed = on;
	strcpy(status, on ? "1" : "0");
	return Publish(status, strlen(status), MQTT.topic.will, MQTT.qos.will, 1);
}

uint8_t MQTT_Subscribe(void) {
	int count = 1, count_rx;
	MQTTString topicFilters[] = {MQTTString_initializer};
	int grantedQos, qoss_rx[count], qoss[] = {MQTT.qos.command};
	unsigned char buf[100];
	int len, buflen = sizeof(buf);
	mqtt_handler_t d;

	topicFilters[0].cstring = MQTT.topic.command;
	len = MQTTSerialize_subscribe(buf, buflen, 0, ++MQTT.packetid, count,
			topicFilters, qoss);

	if (!Upload(buf, len, SUBACK, MQTT_UPLOAD_MS))
		return 0;

	if (!MQTTDeserialize_suback(&(d.packetid), count, &count_rx, qoss_rx, buf,
			buflen)) {
		printf("MQTT:Subscribe failed\n");
		return 0;
	}

	if (MQTT.packetid != d.packetid) {
		printf("MQTT:SUBACK packet mismatched\n");
		return 0;
	}
	printf("MQTT:SUBACK\n");

	grantedQos = qoss_rx[0];
	if (MQTT.qos.command != grantedQos) {
		printf("MQTT:Granted QoS != %d, %d\n", MQTT.qos.command, grantedQos);
		// return 0;
	}

	MQTT.subscribed = 1;

	printf("MQTT:Subscribed\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

uint8_t MQTT_Unsubscribe(void) {
	MQTTString topicFilters = MQTTString_initializer;
	unsigned char buf[50];
	int len, buflen = sizeof(buf);
	int count = 1;
	mqtt_handler_t d;

	topicFilters.cstring = MQTT.topic.command;
	len = MQTTSerialize_unsubscribe(buf, buflen, 0, ++MQTT.packetid, count,
			&topicFilters);

	if (!Upload(buf, len, UNSUBACK, MQTT_UPLOAD_MS))
		return 0;

	if (!MQTTDeserialize_unsuback(&(d.packetid), buf, buflen)) {
		printf("MQTT:Unsubscribe failed\n");
		return 0;
	}

	if (MQTT.packetid != d.packetid) {
		printf("MQTT:UNSUBACK packet mismatched\n");
		return 0;
	}
	printf("MQTT:UNSUBACK\n");

	printf("MQTT:Unsubscribed\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

uint8_t MQTT_Connect(void) {
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char sessionPresent, connack_rc;
	char clientId[20], status[] = "0";
	unsigned char buf[256];
	int len, buflen = sizeof(buf);

	MQTT.subscribed = 0;
	MQTT.willed = 0;

	// generate topics
	sprintf(MQTT.topic.command, "VCU/%lu/CMD", VIN_VALUE);
	sprintf(MQTT.topic.response, "VCU/%lu/RSP", VIN_VALUE);
	sprintf(MQTT.topic.report, "VCU/%lu/RPT", VIN_VALUE);
	sprintf(MQTT.topic.will, "VCU/%lu/STS", VIN_VALUE);
	sprintf(clientId, "VCU-%lu", VIN_VALUE);

	// subscribe
	data.clientID.cstring = clientId;
	data.cleansession = !MQTT_PERSIST_SESSION;
	data.keepAliveInterval = MQTT_KEEPALIVE_S;
	data.username.cstring = MQTT_USERNAME;
	data.password.cstring = MQTT_PASSWORD;

	data.willFlag = 1;
	data.will.retained = 1;
	data.will.qos = MQTT.qos.will;
	data.will.topicName.cstring = MQTT.topic.will;
	data.will.message.cstring = status;

	len = MQTTSerialize_connect(buf, buflen, &data);

	if (!Upload(buf, len, CONNACK, MQTT_UPLOAD_MS))
		return 0;

	if (!MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) ||
			connack_rc != 0) {
		printf("MQTT:Connect failed, RC %d\n", connack_rc);
		return 0;
	}
	printf("MQTT:CONNACK\n");

	if (sessionPresent)
		MQTT.subscribed = 1;

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

	if ((_GetTickMS() - MQTT.tick) <= (MQTT_KEEPALIVE_S * 1000))
		return 1;

	len = MQTTSerialize_pingreq(buf, buflen);

	if (!Upload(buf, len, PINGRESP, MQTT_UPLOAD_MS))
		return 0;

	printf("MQTT:Pinged\n");
	MQTT.tick = _GetTickMS();
	return 1;
}

uint8_t MQTT_GotPublish(void) {
	unsigned char buf[256], *dst;
	int len, buflen = sizeof(buf);
	mqtt_handler_t *d = &(MQTT.rx.d);
	MQTTString topic;

	if (MQTT.rx.pending)
		return 0;

	d->packettype = MQTTPacket_read(buf, buflen, Simcom_GetData);
	if (d->packettype != PUBLISH)
		return 0;

	if (!MQTTDeserialize_publish(&(d->dup), &(d->qos), &(d->retained),
			&(d->packetid), &topic, &dst, &len, buf, buflen))
		return 0;

	printf("MQTT:Received %d bytes\n", len);
	if (len == 0)
		return 0;

	sprintf(d->topic, "%.*s", topic.lenstring.len, topic.lenstring.data);
	if (strcmp(d->topic, MQTT.topic.command) != 0)
		return 0;

	if (!CMD_ValidateContent(dst, len))
		return 0;

	memcpy(&(MQTT.rx.command), dst, len); _DelayMS(2);
	MQTT.rx.pending = 1;
	return 1;
}

uint8_t MQTT_AckPublish(command_t *cmd) {
	unsigned char buf[256], packettype;
	int len, buflen = sizeof(buf);
	mqtt_handler_t *d = &(MQTT.rx.d);
	unsigned short packetid;

	if (d->qos == 1) {
		len = MQTTSerialize_puback(buf, buflen, d->packetid);
		if (!Upload(buf, len, 0, MQTT_UPLOAD_MS))
			return 0;
	}

	else if (d->qos == 2) {
		len = MQTTSerialize_ack(buf, buflen, PUBREC, 0, d->packetid);
		if (!Upload(buf, len, PUBREL, MQTT_UPLOAD_MS))
			return 0;

		if (!MQTTDeserialize_ack(&packettype, &(d->dup), &packetid, buf, buflen))
			return 0;

		if (d->packetid != packetid) {
			printf("MQTT:PUBREL packet mismatched\n");
			return 0;
		}
		printf("MQTT:PUBREL packet mismatched\n");

		len = MQTTSerialize_pubcomp(buf, buflen, d->packetid);
		if (!Upload(buf, len, 0, MQTT_UPLOAD_MS))
			return 0;
	}

	// send ACK (application-level)
	if (!Publish(PREFIX_ACK, 2, MQTT.topic.response, MQTT.qos.response, 1))
		return 0;

	if (!CMD_ValidateCode(&(MQTT.rx.command)))
		return 0;

	memcpy(cmd, &(MQTT.rx.command), sizeof(command_t)); _DelayMS(2);
	MQTT.rx.pending = 0;
	return 1;
}

uint8_t MQTT_GotCommand(void) {
	return MQTT.rx.pending;
}

void MQTT_FlushCommand(void) {
	memset(&(MQTT.rx), 0, sizeof(mqtt_rx_t));
}

uint8_t MQTT_Subscribed(void) {
	return MQTT.subscribed;
}

uint8_t MQTT_Willed(void) {
	return MQTT.willed;
}

/* Private functions implementation
 * -------------------------------------------*/
static uint8_t Publish(void *payload, uint16_t payloadlen, char *topic, int qos, uint8_t retained) {
	MQTTString topicName = MQTTString_initializer;
	unsigned char buf[512];
	int len, buflen = sizeof(buf);
	mqtt_handler_t d;
	uint8_t reply = 0;

	topicName.cstring = topic;
	len = MQTTSerialize_publish(buf, buflen, 0, qos, (unsigned char) retained, ++MQTT.packetid, topicName, payload, payloadlen);

	if (qos)
		reply = (qos == 1) ? PUBACK : PUBREC;

	if (!Upload(buf, len, reply, MQTT_UPLOAD_MS))
		return 0;

	if (qos == 1) {
		if (!MQTTDeserialize_ack(&(d.packettype), &(d.dup), &(d.packetid), buf, buflen))
			return 0;

		if (MQTT.packetid != d.packetid) {
			printf("MQTT:PUBACK packet mismatched\n");
			return 0;
		}
		printf("MQTT:PUBACK\n");
	}

	else if (qos == 2) {
		if (!MQTTDeserialize_ack(&(d.packettype), &(d.dup), &(d.packetid), buf, buflen))
			return 0;

		if (MQTT.packetid != d.packetid) {
			printf("MQTT:PUBREC packet mismatched\n");
			return 0;
		}
		printf("MQTT:PUBREC\n");

		len = MQTTSerialize_pubrel(buf, buflen, 0, MQTT.packetid);
		if (!Upload(buf, len, PUBCOMP, MQTT_UPLOAD_MS))
			return 0;

		if (!MQTTDeserialize_ack(&(d.packettype), &(d.dup), &(d.packetid), buf, buflen))
			return 0;

		if (MQTT.packetid != d.packetid) {
			printf("MQTT:PUBCOMP packet mismatched\n");
			return 0;
		}
		printf("MQTT:PUBCOMP\n");
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
