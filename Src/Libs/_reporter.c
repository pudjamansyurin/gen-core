/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"
#include "_crc.h"

// variable list
response_t response;
report_t report;

// function list
void Reporter_Reset(frame_t frame) {
	// set default data
	Reporter_Set_Prefix(0x4047);
	Reporter_Set_UnitID(354313);

	if (frame == FRAME_RESPONSE) {
		// header response
		response.header.crc = 0;
		response.header.size = 0;
		response.header.frame_id = FRAME_RESPONSE;

		// body response
		strcpy(response.data.message, "");
	} else {
		// header report
		report.header.crc = 0;
		report.header.size = 0;
		report.header.frame_id = frame;

		// body req
		if (frame >= FRAME_SIMPLE) {
			report.data.req.seq_id = 0;
			report.data.req.rtc_send_datetime = 0;
			report.data.req.rtc_log_datetime = 0;
			report.data.req.driver_id = 1;
			report.data.req.events_group = 0;
			report.data.req.speed = 1;
		}

		// body opt
		if (frame == FRAME_FULL) {
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

void Reporter_Set_Prefix(uint16_t prefix) {
	// @G = 0x4047
	response.header.prefix = prefix;
	report.header.prefix = prefix;

}

void Reporter_Set_UnitID(uint32_t unitId) {
	response.header.unit_id = unitId;
	report.header.unit_id = unitId;
}

void Reporter_Set_Odometer(uint32_t odom) {
	report.data.opt.odometer = odom;
	Flash_Save_Odometer(odom);
}

void Reporter_Set_GPS(gps_t *hgps) {
	// parse gps data
	// FIXME handle float to S32 conversion
	// FIXME check GPS data on the road and the calibration
	report.data.opt.gps.latitude = hgps->latitude;
	report.data.opt.gps.longitude = hgps->longitude;
	report.data.opt.gps.hdop = hgps->dop_h;
	// FIXME i should be updated based on GPS data
	report.data.opt.gps.heading = hgps->variation;
}

void Reporter_Set_Speed(gps_t *hgps) {
	float d_distance;
	// parse gps data
	// FIXME use real speed calculation
	// calculate speed from GPS data
	report.data.req.speed = gps_to_speed(hgps->speed, gps_speed_kph);
	// change odometer from GPS speed variation
	d_distance = (gps_to_speed(hgps->speed, gps_speed_mps) * REPORT_INTERVAL_SIMPLE);
	// save odometer to flash (non-volatile)
	Reporter_Set_Odometer(Flash_Get_Odometer() + d_distance);

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

void Reporter_Set_Header(frame_t frame) {
	if (frame == FRAME_RESPONSE) {
		//Reconstruct the header
		response.header.size = sizeof(response.header.frame_id) +
				sizeof(response.header.unit_id) +
				sizeof(response.data.code) +
				strlen(response.data.message);
		response.header.frame_id = FRAME_RESPONSE;
		response.header.crc = CRC_Calculate8(
				(uint8_t*) &(response.header.size),
				response.header.size + sizeof(response.header.size), 1);

	} else {
		// parse newest rtc datetime
		report.data.req.rtc_log_datetime = RTC_Read();
		report.data.req.seq_id++;
		report.header.frame_id = frame;

		report.header.size = sizeof(report.header.frame_id) +
				sizeof(report.header.unit_id) +
				sizeof(report.data.req);

		//Reconstruct the header
		if (frame == FRAME_FULL) {
			report.header.size += sizeof(report.data.opt);
		}

		// it will be recalculated when sending the payload
		report.header.crc = 0;
		report.data.req.rtc_send_datetime = 0;
	}
}

