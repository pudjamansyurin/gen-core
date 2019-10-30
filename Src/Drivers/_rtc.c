/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

#include "_rtc.h"

extern RTC_HandleTypeDef hrtc;

void RTC_Read_RAW(timestamp_t * timestamp) {
	// get the RTC
	HAL_RTC_GetTime(&hrtc, &(timestamp->time), RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &(timestamp->date), RTC_FORMAT_BIN);
}

void RTC_Read(char *dateTime) {
	timestamp_t timestamp;

	// get the RTC
	RTC_Read_RAW(&timestamp);

	// combine RTC datetime
	sprintf(dateTime, "20%02d%02d%02d%02d%02d%02d", timestamp.date.Year, timestamp.date.Month, timestamp.date.Date, timestamp.time.Hours,
			timestamp.time.Minutes, timestamp.time.Seconds);
}

void RTC_Write(char *dateTime) {
	// format dateTime: YYYYMMDDHHMMSS
	char Y[2], M[2], D[2], H[2], I[2], S[2];
	timestamp_t timestamp;

	// parsing
	strncpy(Y, dateTime + 2, 2);
	strncpy(M, dateTime + 4, 2);
	strncpy(D, dateTime + 6, 2);
	strncpy(H, dateTime + 8, 2);
	strncpy(I, dateTime + 10, 2);
	strncpy(S, dateTime + 12, 2);

	// assing to object
	// FIXME day is ignored
	timestamp.date.WeekDay = RTC_WEEKDAY_SUNDAY;
	timestamp.date.Year = atoi(Y);
	timestamp.date.Month = atoi(M);
	timestamp.date.Date = atoi(D);
	timestamp.time.Hours = atoi(H);
	timestamp.time.Minutes = atoi(I);
	timestamp.time.Seconds = atoi(S);
	timestamp.time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestamp.time.StoreOperation = RTC_STOREOPERATION_RESET;

	// set the RTC
	HAL_RTC_SetTime(&hrtc, &(timestamp.time), RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &(timestamp.date), RTC_FORMAT_BIN);
}

uint8_t RTC_Offset(uint8_t hour, int offset) {
	return (hour + offset) > 23 ? ((hour + offset) - 24) : (hour + offset);
}
