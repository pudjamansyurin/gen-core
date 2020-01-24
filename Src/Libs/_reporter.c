/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"

// variable list
extern timestamp_t DB_ECU_TimeStamp;
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
			report.data.req.rtc_datetime = 0;
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
			// FIXME get my real value using ADC
			report.data.opt.bat_voltage = 2600 / 13;
			report.data.opt.report_range = 0;
			report.data.opt.report_battery = 99;
			report.data.opt.trip_a = 0;
			report.data.opt.trip_b = 1;
		}
	}
}

void Reporter_Set_Prefix(uint16_t prefix) {
	// @G = 0x4047
	response.header.prefix = prefix;
	report.header.prefix = prefix;
}

void Reporter_Set_UnitID(uint64_t unitId) {
	response.header.unit_id = unitId;
	report.header.unit_id = unitId;
}

void Reporter_Set_Odometer(uint32_t odom) {
	report.data.opt.odometer = odom;
	Flash_Save_Odometer(odom);
}

void Reporter_Set_GPS(gps_t *hgps) {
	// parse gps data
	if (hgps->fix > 0) {
		// FIXME handle float to S32 conversion
		report.data.opt.gps.latitude = hgps->latitude;
		report.data.opt.gps.longitude = hgps->longitude;
		report.data.opt.gps.hdop = hgps->dop_h;
		// FIXME i should be updated based on GPS data
		report.data.opt.gps.heading = 65;
	}
}

void Reporter_Set_Speed(gps_t *hgps) {
	float d_distance;
	// parse gps data
	if (hgps->fix > 0) {
		// FIXME use real speed calculation
		// calculate speed from GPS data
		report.data.req.speed = gps_to_speed(hgps->speed, gps_speed_kph);
		// change odometer from GPS speed variation
		d_distance = (gps_to_speed(hgps->speed, gps_speed_mps) * REPORT_INTERVAL_SIMPLE);
		// save odometer to flash (non-volatile)
		Reporter_Set_Odometer(Flash_Get_Odometer() + d_distance);
	}
}

void Reporter_Set_Event(uint64_t event_id, uint8_t bool) {
	if (bool & 1) {
		// set
		SetBitOf(report.data.req.events_group, BSP_Bit_Pos(event_id));
	} else {
		// clear
		ClearBitOf(report.data.req.events_group, BSP_Bit_Pos(event_id));
	}
}

void Reporter_Set_Header(frame_t frame) {
	if (frame == FRAME_RESPONSE) {
		//Reconstruct the header
		// FIXME use CRC hardware calculation
		response.header.crc = 0;
		response.header.size = sizeof(response.header.frame_id) +
				sizeof(response.header.unit_id) +
				strlen(response.data.message);
		response.header.frame_id = FRAME_RESPONSE;

	} else {
		// parse newest rtc datetime
		report.data.req.rtc_datetime = RTC_Read();
		report.data.req.seq_id++;

		//Reconstruct the header
		if (frame == FRAME_SIMPLE) {
			// FIXME use CRC hardware calculation
			report.header.crc = 0;
			report.header.size = sizeof(report.header.frame_id) +
					sizeof(report.header.unit_id) +
					sizeof(report.data.req);
		} else {
			// FIXME use CRC hardware calculation
			report.header.crc = 0;
			report.header.size = sizeof(report.header.frame_id) +
					sizeof(report.header.unit_id) +
					sizeof(report.data.req) +
					sizeof(report.data.opt);
		}
		report.header.frame_id = frame;
	}
}

