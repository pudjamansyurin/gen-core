/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#include "_reporter.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	char datetime[15];
	char longitude[20];
	char latitude[20];
	char heading[5];
	char hdop[5];
} report_data_gps_t;

typedef struct {
	char datetime_rtc[15];
	char datetime_sending[15];
	report_id_t reportd_id;
	uint32_t odometer;
	uint8_t status_input;
	char speed[5];
	uint8_t status_output;
	uint16_t analog_input;
	char driver_id[17];
	int16_t temp_sensor1;
	int16_t temp_sensor2;
	char message[REPORT_MESSAGE_LENGTH];
	report_data_gps_t gps;
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

	strcpy(report.data.datetime_rtc, "");
	strcpy(report.data.datetime_sending, "");
	strcpy(report.data.speed, "0");
	strcpy(report.data.driver_id, "31313");
	strcpy(report.data.message, "");
	report.data.reportd_id = REPORT_OK;
	report.data.odometer = Flash_Get_Odometer();
	report.data.status_input = 0;
	report.data.status_output = 0;
	report.data.analog_input = 0;
	report.data.temp_sensor1 = 2000;
	report.data.temp_sensor2 = 2000;

	strcpy(report.data.gps.datetime, "");
	strcpy(report.data.gps.longitude, "0");
	strcpy(report.data.gps.latitude, "0");
	strcpy(report.data.gps.heading, "108");
	strcpy(report.data.gps.hdop, "0");
	strcpy(report.data.gps.datetime, "");
}

void Reporter_Set_Message(char *msg) {
	sprintf(report.data.message, "%s\x1E", msg);
}

void Reporter_Set_Report_ID(report_id_t reportID) {
	report.data.reportd_id = reportID;
}

void Reporter_Read_GPS(gps_t *hgps) {
	// parse gps data
	if (hgps->fix > 0) {
		Reporter_Set_Odometer(report.data.odometer + (gps_to_speed(hgps->speed, gps_speed_mps) * REPORT_INTERVAL));
		ftoa(hgps->latitude, report.data.gps.latitude, 6);
		ftoa(hgps->longitude, report.data.gps.longitude, 6);
		ftoa(hgps->dop_h, report.data.gps.hdop, 3);
		ftoa(gps_to_speed(hgps->speed, gps_speed_kph), report.data.speed, 1);
		sprintf(report.data.gps.datetime, "20%02d%02d%02d%02d%02d%02d", hgps->year, hgps->month, hgps->date,
				RTC_Offset(hgps->hours, GMT_TIME),
				hgps->minutes, hgps->seconds);
	}
}

void Reporter_Set_Sending_Time(void) {
	RTC_Read(report.data.datetime_sending);

	// FIXME datetime sending should updated
	//	str_replace(PAYLOAD, "_SENDING_TIME_", report.data.datetime_sending);
}

void Reporter_Set_Payload(void) {
	// parse rtc datetime
	RTC_Read(report.data.datetime_rtc);
	//Reconstruct the data
	sprintf(POSITION_DATA, "%s,%s,_SENDING_TIME_,%s,%s,"
			"%s,%d,%lu,%s,%d,"
			"%s,%d,%d,%s,%d,"
			"%d,%s\r\n", report.data.gps.datetime, report.data.datetime_rtc, report.data.gps.longitude, report.data.gps.latitude,
			report.data.gps.heading, report.data.reportd_id, report.data.odometer, report.data.gps.hdop, report.data.status_input,
			report.data.speed, report.data.status_output, report.data.analog_input, report.data.driver_id, report.data.temp_sensor1,
			report.data.temp_sensor2, report.data.message);
	//Reconstruct the header
	report.header.length = strlen(POSITION_DATA);
	report.header.seq_id++;
	sprintf(POSITION_HEADER, "%s,%d,%d,%d,%s", report.header.prefix, report.header.crc, report.header.length, report.header.seq_id,
			report.header.unit_id);
	//Reconstruct the position
	sprintf(PAYLOAD, "%s,%s", POSITION_HEADER, POSITION_DATA);
}

void Reporter_Set_Odometer(uint32_t odom) {
	report.data.odometer = odom;
	Flash_Save_Odometer(odom);
}
