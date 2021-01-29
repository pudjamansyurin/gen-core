/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Drivers/_crc.h"
#include "Drivers/_simcom.h"
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"
#include "Libs/_eeprom.h"
#include "Nodes/BMS.h"

/* Private functions declarations -------------------------------------------*/
static void RPT_SetHeader(FRAME_TYPE frame, void *payload, uint32_t unit_id);

/* Public functions implementation -------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report, vcu_data_t *vcu, bms_data_t *bms, hbar_data_t *hbar) {
	RPT_SetHeader(frame, report, vcu->unit_id);

	// Reconstruct the body
	report->data.req.vcu.driver_id = vcu->driver_id;
	report->data.req.vcu.events_group = vcu->events;
	report->data.req.vcu.rtc.log = RTC_Read();

	for (uint8_t i = 0; i < BMS_COUNT ; i++) {
		report->data.req.bms.pack[i].id = bms->pack[i].id;
		report->data.req.bms.pack[i].voltage = bms->pack[i].voltage * 100;
		report->data.req.bms.pack[i].current = (bms->pack[i].current + 50) * 100;
	}

	if (frame == FR_FULL) {
		report->data.opt.vcu.vehicle = (int8_t) vcu->state.vehicle;

		report->data.opt.vcu.gps.latitude = (int32_t) (vcu->gps.latitude * 10000000);
		report->data.opt.vcu.gps.longitude = (int32_t) (vcu->gps.longitude * 10000000);
		report->data.opt.vcu.gps.altitude = (uint32_t) vcu->gps.altitude;
		report->data.opt.vcu.gps.hdop = (uint8_t) (vcu->gps.dop_h * 10);
		report->data.opt.vcu.gps.vdop = (uint8_t) (vcu->gps.dop_v * 10);
		report->data.opt.vcu.gps.heading = (uint8_t) (vcu->gps.heading / 2);
		report->data.opt.vcu.gps.sat_in_use = (uint8_t) vcu->gps.sat_in_use;

		report->data.opt.vcu.speed = vcu->speed;
		report->data.opt.vcu.odometer = vcu->odometer;

		report->data.opt.vcu.trip.a = hbar->trip[HBAR_M_TRIP_A];
		report->data.opt.vcu.trip.b = hbar->trip[HBAR_M_TRIP_B];
		report->data.opt.vcu.report.range = hbar->report[HBAR_M_REPORT_RANGE];
		report->data.opt.vcu.report.efficiency = hbar->report[HBAR_M_REPORT_AVERAGE];

		report->data.opt.vcu.signal = SIM.signal;
		report->data.opt.vcu.bat = vcu->bat / 18;

		for (uint8_t i = 0; i < BMS_COUNT ; i++) {
			report->data.opt.bms.pack[i].soc = bms->pack[i].soc * 100;
			report->data.opt.bms.pack[i].temperature = (bms->pack[i].temperature + 40) * 10;
		}

		// test data
		memcpy(&(report->data.test.motion), &(vcu->motion), sizeof(motion_t));
		memcpy(&(report->data.test.task), &(vcu->task), sizeof(rtos_task_t));
	}
}

void RPT_ResponseCapture(response_t *response, uint32_t *unit_id) {
	RPT_SetHeader(FR_RESPONSE, response, *unit_id);
}

void RPT_FrameDecider(uint8_t backup, FRAME_TYPE *frame) {
	static uint8_t frameDecider = 0;

	if (backup) {
		*frame = FR_FULL;
		frameDecider = 0;
	} else {
		if (++frameDecider < (RPT_FRAME_FULL / RPT_INTERVAL_NORMAL ))
			*frame = FR_SIMPLE;
		else {
			*frame = FR_FULL;
			frameDecider = 0;
		}
	}
}

uint8_t RPT_PayloadPending(payload_t *payload) {
	// Handle full buffer
	if (payload->type == PAYLOAD_REPORT)
		if (_osThreadFlagsWait(NULL, EVT_IOT_DISCARD, osFlagsWaitAny, 0))
			payload->pending = 0;

	// Check logs
	if (!payload->pending)
		if (osMessageQueueGet(*(payload->pQueue), payload->pPayload, NULL, 0) == osOK)
			payload->pending = 1;

	return payload->pending;
}

uint8_t RPT_WrapPayload(payload_t *payload) {
	header_t *header = (header_t*) (payload->pPayload);

	// Re-calculate CRC
	if (payload->type == PAYLOAD_REPORT)
		((report_t*) payload->pPayload)->data.req.vcu.rtc.send = RTC_Read();

	header->crc = CRC_Calculate8(
			(uint8_t*) &(header->size),
			header->size + sizeof(header->size),
			0);

	// Calculate final size
	payload->size = sizeof(header->prefix)	+ sizeof(header->crc) + sizeof(header->size) + header->size;

	return payload->size;
}

/* Private functions implementation -------------------------------------------*/
static void RPT_SetHeader(FRAME_TYPE frame, void *payload, uint32_t unit_id) {
	header_t *header = (header_t*) payload;

	memcpy(header->prefix, PREFIX_REPORT, 2);
	header->unit_id = unit_id;
	header->frame_id = frame;
	header->size = sizeof(header->frame_id) + sizeof(header->unit_id);

	if (frame != FR_RESPONSE) {
		report_t *report = (report_t*) payload;
		header->size += sizeof(report->data.req);

		if (frame == FR_FULL)
			header->size += sizeof(report->data.opt) + sizeof(report->data.test);
	} else {
		response_t *response = (response_t*) payload;

		header->size += sizeof(response->data.code) + strlen(response->data.message);
	}
}
