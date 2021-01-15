/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Libs/_at.h"

/* Private variables ----------------------------------------------------------*/
extern osMutexId_t RtcMutexHandle;

/* Private variables ----------------------------------------------------------*/
static rtc_t rtc;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void RTC_Init(RTC_HandleTypeDef *hrtc) {
	rtc.h.rtc = hrtc;
}

timestamp_t RTC_Decode(uint64_t dateTime) {
	// format dateTime: YYMMDDHHmmssE
	uint8_t dt[7];
	timestamp_t timestamp;
	uint64_t tot = 0, mul;

	// parsing to timestamp
	for (int i = 0; i <= 6; i++) {
		if (i < 6)
			mul = pow(10, (11 - (2 * i)));
		else
			mul = 1;

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

uint64_t RTC_Encode(timestamp_t timestamp) {
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
		if (i < 6)
			mul = pow(10, (11 - (2 * i)));
		else
			mul = 1;
		tot += dt[i] * mul;
	}

	return tot;
}

uint64_t RTC_Read(void) {
	timestamp_t timestamp;

	// get the RTC
	RTC_ReadRaw(&timestamp);

	// encode timestamp to datetime
	return RTC_Encode(timestamp);
}

void RTC_ReadRaw(timestamp_t *timestamp) {
	// get the RTC
	lock();
	HAL_RTC_GetTime(rtc.h.rtc, &timestamp->time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(rtc.h.rtc, &timestamp->date, RTC_FORMAT_BIN);
	timestamp->tzQuarterHour = 0;
	unlock();
}

void RTC_Write(uint64_t dateTime, datetime_t *dt) {
	timestamp_t timestamp;

	// decode datetime to timestamp
	timestamp = RTC_Decode(dateTime);

	// set the RTC
	RTC_WriteRaw(&timestamp, dt);
}

void RTC_WriteRaw(timestamp_t *timestamp, datetime_t *dt) {
	// add extra property
	timestamp->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	timestamp->time.StoreOperation = RTC_STOREOPERATION_RESET;

	// set the RTC
	lock();
	HAL_RTC_SetTime(rtc.h.rtc, &timestamp->time, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(rtc.h.rtc, &timestamp->date, RTC_FORMAT_BIN);
	unlock();

	// save calibration date
	// source from server is always considered as valid
	dt->calibration = timestamp->date;
	dt->timestamp = *timestamp;
}

uint8_t RTC_IsDaylight(timestamp_t timestamp) {
	return (timestamp.time.Hours >= 5 && timestamp.time.Hours <= 16);
}

uint8_t RTC_NeedCalibration(datetime_t *dt) {
	RTC_ReadRaw(&(dt->timestamp));

	// Compare
	return (dt->calibration.Year != dt->timestamp.date.Year ||
			dt->calibration.Month != dt->timestamp.date.Month ||
			dt->calibration.Date != dt->timestamp.date.Date);
}

void RTC_CalibrateWithSimcom(datetime_t *dt) {
	timestamp_t timestamp;

	if (AT_Clock(ATR, &timestamp))
		if (timestamp.date.Year >= VCU_BUILD_YEAR)
			RTC_WriteRaw(&timestamp, dt);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
	osMutexAcquire(RtcMutexHandle, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(RtcMutexHandle);
}
