/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"

// variable list
extern timestamp_t DB_ECU_TimeStamp;
report_t report;

// function list
void Reporter_Reset(void) {
	// set default data
	report.header.prefix = 0x4047;
	report.header.crc = 0;
	report.header.length = 0;
	report.header.seq_id = 0;
	report.header.frame_id = 3;
	report.header.unit_id = 354453;

	report.data.rtc_datetime = 0;
	report.data.driver_id = 1;
	report.data.events_group = 0;
	report.data.gps.longitude = 1;
	report.data.gps.latitude = 0;
	report.data.gps.hdop = 1;
	report.data.gps.heading = 0;
	report.data.speed = 1;
	report.data.odometer = Flash_Get_Odometer();
	// FIXME get my real value using ADC
	report.data.bat_voltage = 200;
	report.data.report_range = 0;
	report.data.report_battery = 99;
	report.data.trip_a = 0;
	report.data.trip_b = 1;
}

void Reporter_Set_Odometer(uint32_t odom) {
	report.data.odometer = odom;
	Flash_Save_Odometer(odom);
}

void Reporter_Set_GPS(gps_t *hgps) {
	float d_distance;
	// parse gps data
	if (hgps->fix > 0) {
		// FIXME handle float to S32 conversion
		report.data.gps.latitude = hgps->latitude;
		report.data.gps.longitude = hgps->longitude;
		report.data.gps.hdop = hgps->dop_h;
		// FIXME i should be updated based on GPS data
		report.data.gps.heading = 65;

		// FIXME use real speed calculation
		// calculate speed from GPS data
		report.data.speed = gps_to_speed(hgps->speed, gps_speed_kph);
		// change odometer from GPS speed variation
		d_distance = (gps_to_speed(hgps->speed, gps_speed_mps) * REPORT_INTERVAL);
		// save odometer to flash (non-volatile)
		Reporter_Set_Odometer(Flash_Get_Odometer() + d_distance);
	}
}

void Reporter_Set_Event(uint64_t event_id, uint8_t bool) {
	if (bool & 1) {
		// set
		SetBitOf(report.data.events_group, BSP_Bit_Pos(event_id));
	} else {
		// clear
		ClearBitOf(report.data.events_group, BSP_Bit_Pos(event_id));
	}
}

void Reporter_Set_Frame(void) {
	// parse rtc datetime
	report.data.rtc_datetime = RTC_Read();

	//Reconstruct the header
	report.header.seq_id++;
	report.header.length = sizeof(report.header.seq_id) +
			sizeof(report.header.frame_id) +
			sizeof(report.header.unit_id) +
			sizeof(report.data);
	// FIXME use CRC hardware calculation
	report.header.crc = 0;
}

