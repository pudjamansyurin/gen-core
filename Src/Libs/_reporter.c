/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	float longitude;
	float latitude;
	float hdop;
	uint16_t heading;
} report_data_gps_t;

typedef struct {
	report_id_t id;
	uint64_t datetime_rtc;
	report_data_gps_t gps;
	uint16_t speed;
	uint32_t odometer;

	uint8_t status_input;
	uint8_t status_output;
	uint16_t analog_input;
	char driver_id[17];
	int16_t temp_sensor1;
	int16_t temp_sensor2;
	char message[REPORT_MESSAGE_LENGTH];
} report_data_t;

typedef struct {
	char prefix[3];
	uint16_t crc;
	uint16_t length;
	uint16_t seq_id;
	char unit_id[10];
} report_header_t;

typedef struct {
	report_header_t header;
	report_data_t data;
} report_t;

// variable list
// FIXME reduce the using of global variables, or if you can't, handle how tasks interact with them.
char POSITION_HEADER[REPORT_POS_HEADER_LENGTH];
char POSITION_DATA[REPORT_POS_DATA_LENGTH];
char PAYLOAD[REPORT_POS_HEADER_LENGTH + REPORT_POS_DATA_LENGTH];
extern timestamp_t DB_ECU_TimeStamp;
report_t report;

// function list
void Reporter_Reset(void) {
	// set initial data
	strcpy(report.header.prefix, "@P");
	strcpy(report.header.unit_id, "354453");
	report.header.crc = 0;
	report.header.length = 0;
	report.header.seq_id = 0;

	report.data.id = REPORT_OK;
	report.data.datetime_rtc = 0;
	report.data.gps.longitude = 0;
	report.data.gps.latitude = 0;
	report.data.gps.hdop = 0;
	report.data.gps.heading = 0;
	report.data.speed = 0;
	report.data.odometer = Flash_Get_Odometer();

	strcpy(report.data.driver_id, "31313");
	strcpy(report.data.message, "");
	report.data.status_input = 0;
	report.data.status_output = 0;
	report.data.analog_input = 0;
	report.data.temp_sensor1 = 2000;
	report.data.temp_sensor2 = 2000;

}

void Reporter_Set_Message(char *msg) {
	sprintf(report.data.message, "%s\x1E", msg);
}

void Reporter_Set_Report_ID(report_id_t reportID) {
	report.data.id = reportID;
}

void Reporter_Read_GPS(gps_t *hgps) {
	float d_distance;
	// parse gps data
	if (hgps->fix > 0) {
		report.data.gps.latitude = hgps->latitude;
		report.data.gps.longitude = hgps->longitude;
		report.data.gps.hdop = hgps->dop_h;
		// FIXME i should be updated based on GPS data
		report.data.gps.heading = 270;

		// calculate speed from GPS data
		report.data.speed = gps_to_speed(hgps->speed, gps_speed_kph);
		// change odometer from GPS speed variation
		d_distance = (gps_to_speed(hgps->speed, gps_speed_mps) * REPORT_INTERVAL);
		report.data.odometer = Flash_Get_Odometer() + d_distance;
		// save odometer to flash (non-volatile)
		Reporter_Save_Odometer(report.data.odometer);
	}
}

void Reporter_Set_Payload(void) {
	// parse rtc datetime
	RTC_Read(report.data.datetime_rtc);

	//Reconstruct the data
	sprintf(POSITION_DATA,
			"%d,%12s,%g,%g,%g,%u,	%lu,%s,%s,%d,%d,%s,%d,%d,%s\r\n",
			(uint8_t) report.data.id,
			(uint64_t) report.data.datetime_rtc,
			(float) report.data.gps.longitude,
			(float) report.data.gps.latitude,
			(float) report.data.gps.hdop,
			(uint16_t) report.data.gps.heading,
			(uint16_t) report.data.speed,
			(uint32_t) report.data.odometer,

			report.data.status_input,
			report.data.status_output,
			report.data.analog_input,
			report.data.driver_id,
			report.data.temp_sensor1,
			report.data.temp_sensor2,
			report.data.message);

	//Reconstruct the header
	report.header.length = strlen(POSITION_DATA);
	report.header.seq_id++;
	sprintf(POSITION_HEADER, "%s,%d,%d,%d,%s",
			report.header.prefix,
			report.header.crc,
			report.header.length,
			report.header.seq_id,
			report.header.unit_id);

	//Reconstruct the position
	sprintf(PAYLOAD, "%s,%s", POSITION_HEADER, POSITION_DATA);
}

void Reporter_Save_Odometer(uint32_t odom) {
	Flash_Save_Odometer(odom);
}
