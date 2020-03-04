/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"
#include "_crc.h"

// variable list
report_t report;
response_t response;

// function list
void Reporter_Reset(frame_t frame) {
	// set default data
	// header report
	report.header.prefix = FRAME_PREFIX;
	report.header.crc = 0;
	report.header.size = 0;
	report.header.frame_id = frame;
	Reporter_Set_UnitID(REPORT_UNITID);
	report.header.seq_id = 0;

	if (frame == FRAME_RESPONSE) {
		// header response
		// (copy from report)
		response.header = report.header;

		// body response
		response.data.code = 1;
		strcpy(response.data.message, "");

	} else {
		// header report
		// (already set)

		// body required
		if (frame == FRAME_SIMPLE || frame == FRAME_FULL) {
			report.data.req.rtc_send_datetime = 0;
			report.data.req.rtc_log_datetime = 0;
			report.data.req.driver_id = 1;
			report.data.req.events_group = 0;
			report.data.req.speed = 1;
		}

		// body optional
		if (frame == FRAME_FULL) {
			// FIXME: default value should be zero
			report.data.opt.gps.longitude = 112.6935779 * 10000000;
			report.data.opt.gps.latitude = -7.4337599 * 10000000;
			report.data.opt.gps.hdop = 255;
			report.data.opt.gps.heading = 0;
			report.data.opt.odometer = Flash_Get_Odometer();
			report.data.opt.bat_voltage = 2600 / 13;
			report.data.opt.report_range = 0;
			report.data.opt.report_battery = 99;
			report.data.opt.trip_a = 0x01234567;
			report.data.opt.trip_b = 0x89A;
		}
	}
}

void Reporter_Set_UnitID(uint32_t unitId) {
	response.header.unit_id = unitId;
	report.header.unit_id = unitId;
}

void Reporter_Set_Odometer(uint32_t odom) {
	report.data.opt.odometer = odom;
	Flash_Save_Odometer(odom);
}

void Reporter_Set_GPS(nmea_t *hgps) {
	// parse GPS data
	report.data.opt.gps.latitude = (int32_t) (hgps->latitude * 10000000);
	report.data.opt.gps.longitude = (int32_t) (hgps->longitude * 10000000);
	report.data.opt.gps.hdop = (uint8_t) (hgps->dop_h * 10);
	report.data.opt.gps.heading = (uint8_t) (hgps->variation / 2);
}

void Reporter_Set_Speed(nmea_t *hgps) {
	float d_distance;
	// parse GPS data
	// FIXME use real speed calculation
	// calculate speed from GPS data
	report.data.req.speed = nmea_to_speed(hgps->speed, nmea_speed_kph);
	// change ODOMETER from GPS speed variation
	d_distance = (nmea_to_speed(hgps->speed, nmea_speed_mps) * REPORT_INTERVAL_SIMPLE);
	// save ODOMETER to flash (non-volatile)
	Reporter_Set_Odometer(Flash_Get_Odometer() + d_distance);

}

void Reporter_Set_Events(uint64_t value) {
	report.data.req.events_group = value;
}

void Reporter_Set_Event(uint64_t event_id, uint8_t bool) {
	if (bool & 1) {
		// set
		SetBitOf(report.data.req.events_group, BSP_BitPosition(event_id));
	} else {
		// clear
		ClearBitOf(report.data.req.events_group, BSP_BitPosition(event_id));
	}
}

uint8_t Reporter_Read_Event(uint64_t event_id) {
	return (report.data.req.events_group & event_id) >> BSP_BitPosition(event_id);
}

void Reporter_Capture(frame_t frame) {
	if (frame == FRAME_RESPONSE) {
		//Reconstruct the header
		response.header.seq_id++;
		response.header.frame_id = FRAME_RESPONSE;
		response.header.size = sizeof(response.header.frame_id) +
				sizeof(response.header.unit_id) +
				sizeof(response.header.seq_id) +
				sizeof(response.data.code) +
				strlen(response.data.message);
		response.header.crc = CRC_Calculate8(
				(uint8_t*) &(response.header.size),
				response.header.size + sizeof(response.header.size), 1);

	} else {
		// Reconstruct the body
		report.data.req.rtc_log_datetime = RTC_Read();
		report.data.req.rtc_send_datetime = 0;

		// Reconstruct the header
		report.header.seq_id++;
		report.header.frame_id = frame;
		report.header.size = sizeof(report.header.frame_id) +
				sizeof(report.header.unit_id) +
				sizeof(report.header.seq_id) +
				sizeof(report.data.req);
		// Add opt on FRAME_FULL
		if (frame == FRAME_FULL) {
			report.header.size += sizeof(report.data.opt);
		}
		// CRC will be recalculated when sending the payload
		// (because RTC_Send_Datetime will be changed later)
		report.header.crc = 0;
	}
}

