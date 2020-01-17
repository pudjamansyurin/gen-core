/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

#include <math.h>
#include "_rtc.h"

extern RTC_HandleTypeDef hrtc;

static timestamp_t Datetime_Decode(uint64_t dateTime) {
	// format dateTime: YYMMDDhhmmssi
	uint8_t dt[7];
	uint64_t tot = 0, div;
	timestamp_t timestamp;

	// parsing to timestamp
	for (int i = 0; i <= 6; i++) {
		if (i < 6) {
			// handle : YYMMDDhhmmss
			div = pow(10, (11 - (2 * i)));
			tot += dt[i] * div;
		} else {
			// handle : i
			div = 1;
		}
		dt[i] = ((dateTime - tot) / div);
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

static uint64_t Datetime_Encode(timestamp_t timestamp) {
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

void RTC_Read_RAW(timestamp_t *timestamp) {
	// get the RTC
	HAL_RTC_GetTime(&hrtc, &(timestamp->time), RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &(timestamp->date), RTC_FORMAT_BIN);
}

uint64_t RTC_Read(void) {
	timestamp_t timestamp;

	// get the RTC
	RTC_Read_RAW(&timestamp);

	// encode timestamp to datetime
	return Datetime_Encode(timestamp);
}

void RTC_Write(uint64_t dateTime) {
	timestamp_t timestamp;

	// decode datetime to timestamp
	timestamp = Datetime_Decode(dateTime);

	// add extra property
	timestamp.time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestamp.time.StoreOperation = RTC_STOREOPERATION_RESET;

	// set the RTC
	HAL_RTC_SetTime(&hrtc, &(timestamp.time), RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &(timestamp.date), RTC_FORMAT_BIN);
}

//uint8_t RTC_Offset(uint8_t hour, int offset) {
//	return (hour + offset) > 23 ? ((hour + offset) - 24) : (hour + offset);
//}
