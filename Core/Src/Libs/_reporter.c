/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_reporter.h"
#include "BMS.h"

/* External variables ----------------------------------------------------------*/
extern db_t DB;
extern gps_t GPS;
extern sw_t SW;
extern bms_t BMS;

/* Public functions implementation --------------------------------------------*/
void Report_Init(FRAME_TYPE frame, report_t *report) {
	// set default data
	LOG_StrLn("Reporter:ReportInit");
	// =============== REPORT ==============
	// header report
	report->header.prefix[0] = PREFIX_REPORT[1];
	report->header.prefix[1] = PREFIX_REPORT[0];
	report->header.seq_id = DB.vcu.seq_id.report;
	// (already set)
	// body required
	report->data.req.vcu.driver_id = DRIVER_ID_NONE;
	// body optional
	report->data.opt.vcu.report.range = 99;
	report->data.opt.vcu.report.efficiency = 88;
}

void Response_Init(response_t *response) {
	// set default data
	LOG_StrLn("Reporter:ResponseInit");
	// =============== REPORT ==============
	// header report
	response->header.prefix[0] = PREFIX_REPORT[1];
	response->header.prefix[1] = PREFIX_REPORT[0];
	response->header.seq_id = DB.vcu.seq_id.response;
}

void Report_Capture(FRAME_TYPE frame, report_t *report) {
	// Reconstruct the header
	report->header.seq_id++;
	report->header.unit_id = DB.vcu.unit_id;
	report->header.frame_id = frame;
	report->header.size = sizeof(report->header.frame_id) +
			sizeof(report->header.unit_id) +
			sizeof(report->header.seq_id) +
			sizeof(report->data.req);

	// Reconstruct the body
	report->data.req.vcu.events_group = DB.vcu.events;
	report->data.req.vcu.rtc.log = RTC_Read();
	// BMS data
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		report->data.req.bms.pack[i].id = BMS.d.pack[i].id;
		report->data.req.bms.pack[i].voltage = BMS.d.pack[i].voltage * 100;
		report->data.req.bms.pack[i].current = (BMS.d.pack[i].current + 50) * 100;
	}

	// Add more (if full frame)
	if (frame == FR_FULL) {
		report->header.size += sizeof(report->data.opt);
		// set parameter
		report->data.opt.vcu.gps.latitude = (int32_t) (GPS.latitude * 10000000);
		report->data.opt.vcu.gps.longitude = (int32_t) (GPS.longitude * 10000000);
		report->data.opt.vcu.gps.hdop = (uint8_t) (GPS.dop_h * 10);
		report->data.opt.vcu.gps.heading = (uint8_t) (GPS.heading / 2);

		report->data.opt.vcu.speed = GPS.speed_kph;
		report->data.opt.vcu.odometer = DB.vcu.odometer;
		report->data.opt.vcu.trip.a = SW.runner.mode.sub.trip[SW_M_TRIP_A];
		report->data.opt.vcu.trip.b = SW.runner.mode.sub.trip[SW_M_TRIP_B];

		report->data.opt.vcu.signal_percent = DB.vcu.signal_percent;
		report->data.opt.vcu.bat_voltage = DB.vcu.bat_voltage / 18;

		// BMS data
		for (uint8_t i = 0; i < BMS_COUNT; i++) {
			report->data.opt.bms.pack[i].soc = BMS.d.pack[i].soc;
			report->data.opt.bms.pack[i].temperature = (BMS.d.pack[i].temperature + 40) * 10;
		}
	}
}

void Response_Capture(response_t *response) {
	//Reconstruct the header
	response->header.seq_id++;
	response->header.unit_id = DB.vcu.unit_id;
	response->header.frame_id = FR_RESPONSE;
	response->header.size = sizeof(response->header.frame_id) +
			sizeof(response->header.unit_id) +
			sizeof(response->header.seq_id) +
			sizeof(response->data.code) +
			strlen(response->data.message);
	response->header.crc = CRC_Calculate8(
			(uint8_t*) &(response->header.size),
			response->header.size + sizeof(response->header.size));
}

void Report_ReCalculate(report_t *report) {
	// get current sending date-time
	report->data.req.vcu.rtc.send = RTC_Read();
	// recalculate the CRC
	report->header.crc = CRC_Calculate8(
			(uint8_t*) &(report->header.size),
			report->header.size + sizeof(report->header.size));

}
