/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
//#include "Drivers/_crc.h"
#include "Drivers/_simcom.h"
#include "Libs/_handlebar.h"
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"
#include "Libs/_eeprom.h"
#include "Nodes/BMS.h"

/* Public functions implementation -------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report, vcu_data_t *vcu, bms_data_t *bms, hbar_data_t *hbar) {
	header_t *header = (header_t*) report;

	memcpy(header->prefix, PREFIX_REPORT, 2);

	// Required data
	header->size = sizeof(report->data.req);

	report->data.req.frame_id = frame;
	report->data.req.vcu.log_time = RTC_Read();
	report->data.req.vcu.driver_id = vcu->driver_id;
	report->data.req.vcu.events_group = vcu->events;
	report->data.req.vcu.vehicle = (int8_t) vcu->state.vehicle;
	report->data.req.vcu.uptime = vcu->uptime * 1.111;

	for (uint8_t i = 0; i < BMS_COUNT ; i++) {
		report->data.req.bms.pack[i].id = bms->pack[i].id;
		report->data.req.bms.pack[i].voltage = bms->pack[i].voltage * 100;
		report->data.req.bms.pack[i].current = (bms->pack[i].current + 50) * 100;
	}

	// Optional data
	if (frame == FR_FULL) {
		header->size += sizeof(report->data.opt);

		report->data.opt.vcu.gps.latitude = (int32_t) (vcu->gps.latitude * 10000000);
		report->data.opt.vcu.gps.longitude = (int32_t) (vcu->gps.longitude * 10000000);
		report->data.opt.vcu.gps.altitude = (uint32_t) vcu->gps.altitude;
		report->data.opt.vcu.gps.hdop = (uint8_t) (vcu->gps.dop_h * 10);
		report->data.opt.vcu.gps.vdop = (uint8_t) (vcu->gps.dop_v * 10);
		report->data.opt.vcu.gps.heading = (uint8_t) (vcu->gps.heading / 2);
		report->data.opt.vcu.gps.sat_in_use = (uint8_t) vcu->gps.sat_in_use;

		report->data.opt.vcu.odometer = hbar->trip[HBAR_M_TRIP_ODO] / 1000;
		report->data.opt.vcu.trip.a = hbar->trip[HBAR_M_TRIP_A];
		report->data.opt.vcu.trip.b = hbar->trip[HBAR_M_TRIP_B];
		report->data.opt.vcu.report.range = hbar->report[HBAR_M_REPORT_RANGE];
		report->data.opt.vcu.report.efficiency = hbar->report[HBAR_M_REPORT_AVERAGE];

		report->data.opt.vcu.speed = vcu->speed;
		report->data.opt.vcu.signal = SIM.signal;
		report->data.opt.vcu.bat = vcu->bat / 18;

		for (uint8_t i = 0; i < BMS_COUNT ; i++) {
			report->data.opt.bms.pack[i].soc = bms->pack[i].soc * 100;
			report->data.opt.bms.pack[i].temperature = (bms->pack[i].temperature + 40) * 10;
		}

		// Debug data
		header->size += sizeof(report->data.debug);
		memcpy(&(report->data.debug.motion), &(vcu->motion), sizeof(motion_t));
		memcpy(&(report->data.debug.task), &(vcu->task), sizeof(rtos_task_t));
	}
}

void RPT_ResponseCapture(response_t *response) {
	header_t *header = (header_t*) response;

	memcpy(header->prefix, PREFIX_RESPONSE, 2);

	header->size = sizeof(response->header.code)
					+sizeof(response->header.sub_code)
					+sizeof(response->data.res_code)
					+ strnlen(response->data.message, sizeof(response->data.message)
					);
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
	if (!payload->pending)
		if (osMessageQueueGet(*(payload->pQueue), payload->pPayload, NULL, 0) == osOK)
			payload->pending = 1;

	return payload->pending;
}

uint8_t RPT_WrapPayload(payload_t *payload) {
	header_t *header = (header_t*) (payload->pPayload);

	header->size += sizeof(header->vin) +
			sizeof(header->send_time);
	header->vin = VIN_VALUE;
	header->send_time = RTC_Read();

	//  // Re-calculate CRC
	//  header->crc = CRC_Calculate8(
	//      (uint8_t*) &(header->size),
	//      header->size + sizeof(header->size),
	//      0);

	payload->size = sizeof(header->prefix)	+
			//			sizeof(header->crc) +
			sizeof(header->size) +
			header->size;

	return payload->size;
}
