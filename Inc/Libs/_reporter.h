/*
 * _reporter.h
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

#ifndef REPORTER_H_
#define REPORTER_H_

#include <stdio.h>							// for: sprintf()
#include "_config.h"
#include "_flash.h"
#include "_eeprom.h"
#include "_nmea.h"
#include "_rtc.h"

#define REPORT_POS_HEADER_LENGTH		31
#define REPORT_POS_DATA_LENGTH			663
#define REPORT_MESSAGE_LENGTH				500

// Report ID type
// @formatter:off
typedef enum {
	REPORT_OK 					= 0,
	REPORT_BIKE_FALLING = 1,
	REPORT_BIKE_CRASHED = 2
}	report_id_t;
// @formatter:on

// public function
void Reporter_Reset(void);
void Reporter_Set_Message(char* msg);
void Reporter_Set_Report_ID(report_id_t reportID);
void Reporter_Convert_GPS(gps_t *hgps);
void Reporter_Set_Sending_Time(void);
void Reporter_Set_Payload(void);
void Reporter_Set_Odometer(uint32_t odom);

#endif /* REPORTER_H_ */
