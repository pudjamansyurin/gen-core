/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

#include "_rtc.h"

extern RTC_HandleTypeDef hrtc;

void RTC_Read_RAW(timestamp_t *timestamp) {
	// get the RTC
	HAL_RTC_GetTime(&hrtc, &(timestamp->time), RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &(timestamp->date), RTC_FORMAT_BIN);
}

uint64_t RTC_Read(void) {
	timestamp_t timestamp;

	// get the RTC
	RTC_Read_RAW(&timestamp);

	// combine RTC datetime
	return (100000000000 * timestamp.date.Year +
			1000000000 * timestamp.date.Month +
			10000000 * timestamp.date.Date +
			100000 * timestamp.time.Hours +
			1000 * timestamp.time.Minutes +
			10 * timestamp.time.Seconds +
			1 * timestamp.date.WeekDay
	);
}

void RTC_Write(uint64_t dateTime) {
	// format dateTime: YYMMDDHHMMSS
	uint8_t dt[7];
	timestamp_t timestamp;

	// parsing
	dt[0] = (dateTime / 100000000000);
	dt[1] = (dateTime / 1000000000);
	dt[2] = (dateTime / 10000000);
	dt[3] = (dateTime / 100000);
	dt[4] = (dateTime / 1000);
	dt[5] = (dateTime / 10) - ((dateTime / 1000) * 1000);
	dt[6] = (dateTime / 1) - ((dateTime / 10) * 10);

	strncpy(Y, dateTime + 2, 2);
	strncpy(M, dateTime + 4, 2);
	strncpy(D, dateTime + 6, 2);
	strncpy(H, dateTime + 8, 2);
	strncpy(I, dateTime + 10, 2);
	strncpy(S, dateTime + 12, 2);

	// assing to object
	// FIXME day is ignored
	timestamp.date.Year = dateTime / 100000000000;
	timestamp.date.Month = dateTime / 1000000000;
	timestamp.date.Date = atoi(D);
	timestamp.time.Hours = atoi(H);
	timestamp.time.Minutes = atoi(I);
	timestamp.time.Seconds = atoi(S);
	timestamp.date.WeekDay = RTC_WEEKDAY_SUNDAY;

	timestamp.time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestamp.time.StoreOperation = RTC_STOREOPERATION_RESET;

	// set the RTC
	HAL_RTC_SetTime(&hrtc, &(timestamp.time), RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &(timestamp.date), RTC_FORMAT_BIN);
}

uint8_t RTC_Offset(uint8_t hour, int offset) {
	return (hour + offset) > 23 ? ((hour + offset) - 24) : (hour + offset);
}
