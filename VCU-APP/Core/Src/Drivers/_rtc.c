/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "rtc.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t RtcMutexHandle;
#endif

/* Private variables
 * ----------------------------------------------------------*/
static RTC_HandleTypeDef *prtc = &hrtc;

/* Private functions declaration
 * ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static void RTC_ReadRaw(timestamp_t *timestamp);
static void RTC_WriteRaw(timestamp_t *timestamp);
static timestamp_t RTC_Decode(datetime_t dt);
static datetime_t RTC_Encode(timestamp_t ts);

/* Public functions implementation
 * --------------------------------------------*/
void RTC_Init(void) {
  // nothing here
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

uint8_t RTC_Daylight() {
  timestamp_t ts;

  RTC_ReadRaw(&ts);
  return (ts.time.Hours >= 5 && ts.time.Hours <= 16);
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
  osMutexAcquire(RtcMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
  osMutexRelease(RtcMutexHandle);
#endif
}

static void RTC_ReadRaw(timestamp_t *timestamp) {
  lock();
  HAL_RTC_GetTime(prtc, &timestamp->time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(prtc, &timestamp->date, RTC_FORMAT_BIN);
  timestamp->tzQuarterHour = 0;
  unlock();
}

static void RTC_WriteRaw(timestamp_t *timestamp) {
  // add extra property
  timestamp->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  timestamp->time.StoreOperation = RTC_STOREOPERATION_RESET;

  // set the RTC
  lock();
  HAL_RTC_SetTime(prtc, &timestamp->time, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(prtc, &timestamp->date, RTC_FORMAT_BIN);
  unlock();
}

static timestamp_t RTC_Decode(datetime_t dt) {
  timestamp_t ts;

  ts.date.Year = dt.Year;
  ts.date.Month = dt.Month;
  ts.date.Date = dt.Date;
  ts.time.Hours = dt.Hours;
  ts.time.Minutes = dt.Minutes;
  ts.time.Seconds = dt.Seconds;
  ts.date.WeekDay = dt.WeekDay == 0 ? 7 : dt.WeekDay;

  return ts;
}

static datetime_t RTC_Encode(timestamp_t ts) {
  datetime_t dt;

  dt.Year = ts.date.Year;
  dt.Month = ts.date.Month;
  dt.Date = ts.date.Date;
  dt.Hours = ts.time.Hours;
  dt.Minutes = ts.time.Minutes;
  dt.Seconds = ts.time.Seconds;
  dt.WeekDay = ts.date.WeekDay == 7 ? 0 : ts.date.WeekDay;

  return dt;
}
