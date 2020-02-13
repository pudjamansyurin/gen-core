/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

#include <math.h>
#include "_rtc.h"

extern RTC_HandleTypeDef hrtc;
RTC_DateTypeDef LastCalibrationDate;

static timestamp_t RTC_Decode(uint64_t dateTime) {
	// format dateTime: YYMMDDHHmmssE
	uint8_t dt[7];
	timestamp_t timestamp;
	uint64_t tot = 0, mul;

	// parsing to timestamp
	for (int i = 0; i <= 6; i++) {
		if (i < 6) {
			mul = pow(10, (11 - (2 * i)));
		} else {
			mul = 1;
		}

		dt[i] = (dateTime - tot) / mul;
		tot += (dt[i] * mul);
	}

	// fill to timestamp
	timestamp.date.Year = dt[0];
	timestamp.date.Month = dt[1];
	timestamp.date.Date = dt[2];
	timestamp.time.Hours = dt[3];
	timestamp.time.Minutes = dt[4];
	timestamp.time.Seconds = dt[5];
	timestamp.date.WeekDay = dt[6];

	return timestamp;
}

static uint64_t RTC_Encode(timestamp_t timestamp) {
	uint8_t dt[7];
	uint64_t tot = 0, mul;

	// get from timestamp
	dt[0] = timestamp.date.Year;
	dt[1] = timestamp.date.Month;
	dt[2] = timestamp.date.Date;
	dt[3] = timestamp.time.Hours;
	dt[4] = timestamp.time.Minutes;
	dt[5] = timestamp.time.Seconds;
	dt[6] = timestamp.date.WeekDay;

	// parsing to datetime
	for (int i = 0; i <= 6; i++) {
		if (i < 6) {
			mul = pow(10, (11 - (2 * i)));
		} else {
			mul = 1;
		}
		tot += dt[i] * mul;
	}

	return tot;
}

uint64_t RTC_Read(void) {
	timestamp_t timestamp;

	// get the RTC
	RTC_Read_RAW(&timestamp);

	// encode timestamp to datetime
	return RTC_Encode(timestamp);
}

void RTC_Write(uint64_t dateTime) {
	timestamp_t timestamp;

	// decode datetime to timestamp
	timestamp = RTC_Decode(dateTime);

	// set the RTC
	RTC_Write_RAW(&timestamp);
}

void RTC_Read_RAW(timestamp_t *timestamp) {
	// get the RTC
	HAL_RTC_GetTime(&hrtc, &(timestamp->time), RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &(timestamp->date), RTC_FORMAT_BIN);
}

void RTC_Write_RAW(timestamp_t *timestamp) {
	// add extra property
	timestamp->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestamp->time.StoreOperation = RTC_STOREOPERATION_RESET;

	// save calibration date
	// source from server is always considered as valid
	LastCalibrationDate = timestamp->date;

	// debugging
	//	SWV_SendStr("\nLastCalibrationDate = ");
	//	SWV_SendInt(LastCalibrationDate.Year);
	//	SWV_SendStr("-");
	//	SWV_SendInt(LastCalibrationDate.Month);
	//	SWV_SendStr("-");
	//	SWV_SendInt(LastCalibrationDate.Date);
	//	SWV_SendStrLn("");

	// set the RTC
	HAL_RTC_SetTime(&hrtc, &(timestamp->time), RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &(timestamp->date), RTC_FORMAT_BIN);
}
