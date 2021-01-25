/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"

/* Private variables ----------------------------------------------------------*/
static rtc_t rtc;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void RTC_ReadRaw(timestamp_t *timestamp);
static void RTC_WriteRaw(timestamp_t *timestamp);
static timestamp_t RTC_Decode(datetime_t dt);
static datetime_t RTC_Encode(timestamp_t ts);

/* Public functions implementation --------------------------------------------*/
void RTC_Init(RTC_HandleTypeDef *hrtc, osMutexId_t mutex) {
	rtc.h.rtc = hrtc;
	rtc.h.mutex = mutex;
}

datetime_t RTC_Read(void) {
	timestamp_t ts;

	RTC_ReadRaw(&ts);
	return RTC_Encode(ts);
}

void RTC_Write(datetime_t dt) {
	timestamp_t ts;

	ts = RTC_Decode(dt);
	RTC_WriteRaw(&ts);
}


uint8_t RTC_NeedCalibration(void) {
	timestamp_t ts;

	RTC_ReadRaw(&ts);
	return ts.date.Year < VCU_BUILD_YEAR;
}

void RTC_Calibrate(timestamp_t *ts) {
	if (ts->date.Year >= VCU_BUILD_YEAR)
		RTC_WriteRaw(ts);
}

uint8_t RTC_IsDaylight() {
	timestamp_t ts;

	RTC_ReadRaw(&ts);
	return (ts.time.Hours >= 5 && ts.time.Hours <= 16);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
	osMutexAcquire(rtc.h.mutex, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(rtc.h.mutex);
}

static void RTC_ReadRaw(timestamp_t *timestamp) {
	lock();
	HAL_RTC_GetTime(rtc.h.rtc, &timestamp->time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(rtc.h.rtc, &timestamp->date, RTC_FORMAT_BIN);
	timestamp->tzQuarterHour = 0;
	unlock();
}

static void RTC_WriteRaw(timestamp_t *timestamp) {
	// add extra property
	timestamp->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestamp->time.StoreOperation = RTC_STOREOPERATION_RESET;

	// set the RTC
	lock();
	HAL_RTC_SetTime(rtc.h.rtc, &timestamp->time, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(rtc.h.rtc, &timestamp->date, RTC_FORMAT_BIN);
	unlock();
}


static timestamp_t RTC_Decode(datetime_t dt) {
	//	// format dateTime: YYMMDDHHmmssE
	//	uint8_t dt[7];
	//	timestamp_t timestamp;
	//	uint64_t tot = 0, mul;
	//
	//	// parsing to timestamp
	//	for (int i = 0; i <= 6; i++) {
	//		if (i < 6)
	//			mul = pow(10, (11 - (2 * i)));
	//		else
	//			mul = 1;
	//
	//		dt[i] = (dateTime - tot) / mul;
	//		tot += (dt[i] * mul);
	//	}
	//
	//	// fill to timestamp
	//	timestamp.date.Year = dt[0];
	//	timestamp.date.Month = dt[1];
	//	timestamp.date.Date = dt[2];
	//	timestamp.time.Hours = dt[3];
	//	timestamp.time.Minutes = dt[4];
	//	timestamp.time.Seconds = dt[5];
	//	timestamp.date.WeekDay = dt[6];
	//
	//	return timestamp;

	timestamp_t ts;

	ts.date.Year = dt.Year;
	ts.date.Month = dt.Month;
	ts.date.Date = dt.Date;
	ts.time.Hours = dt.Hours;
	ts.time.Minutes = dt.Minutes;
	ts.time.Seconds = dt.Seconds;
	ts.date.WeekDay = dt.WeekDay;

	return ts;
}

static datetime_t RTC_Encode(timestamp_t ts) {
	//	uint8_t dt[7];
	//	uint64_t tot = 0, mul;
	//
	//	// get from timestamp
	//	dt[0] = timestamp.date.Year;
	//	dt[1] = timestamp.date.Month;
	//	dt[2] = timestamp.date.Date;
	//	dt[3] = timestamp.time.Hours;
	//	dt[4] = timestamp.time.Minutes;
	//	dt[5] = timestamp.time.Seconds;
	//	dt[6] = timestamp.date.WeekDay;
	//
	//	// parsing to datetime
	//	for (int i = 0; i <= 6; i++) {
	//		if (i < 6)
	//			mul = pow(10, (11 - (2 * i)));
	//		else
	//			mul = 1;
	//		tot += dt[i] * mul;
	//	}
	//
	//	return tot;

	datetime_t dt;

	dt.Year = ts.date.Year;
	dt.Month = ts.date.Month;
	dt.Date = ts.date.Date;
	dt.Hours = ts.time.Hours;
	dt.Minutes = ts.time.Minutes;
	dt.Seconds = ts.time.Seconds;
	dt.WeekDay = ts.date.WeekDay;

	return dt;
}
