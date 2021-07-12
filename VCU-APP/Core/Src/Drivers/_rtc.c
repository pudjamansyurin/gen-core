/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_rtc.h"

#include "rtc.h"

/* Exported constants
 * --------------------------------------------*/
#define RTC_CALIBRATION_MINUTES ((uint8_t)60)

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t RtcMutexHandle;
#endif

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint32_t calibTick;
  RTC_HandleTypeDef *prtc;
} rtc_t;

/* Private variables
 * --------------------------------------------*/
static rtc_t RT = {
    .calibTick = 0,
    .prtc = &hrtc,
};

/* Private functions prototype
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);
static void ReadRaw(timestamp_t *timestamp);
static void WriteRaw(timestamp_t *timestamp);
static timestamp_t Decode(datetime_t dt);
static datetime_t Encode(timestamp_t ts);

/* Public functions implementation
 * --------------------------------------------*/
void RTC_Init(void) {
  // nothing here
}

datetime_t RTC_Read(void) {
  timestamp_t ts;

  ReadRaw(&ts);
  return Encode(ts);
}

void RTC_Write(datetime_t dt) {
  timestamp_t ts;

  ts = Decode(dt);
  WriteRaw(&ts);
}

uint8_t RTC_NeedCalibration(void) {
  return !tickIn(RT.calibTick, RTC_CALIBRATION_MINUTES * 60 * 1000);
}

void RTC_Calibrate(timestamp_t *ts) {
  WriteRaw(ts);
  RT.calibTick = tickMs();
}

uint8_t RTC_Daylight() {
  timestamp_t ts;

  ReadRaw(&ts);
  return (ts.time.Hours >= 5 && ts.time.Hours <= 16);
}

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(RtcMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(RtcMutexHandle);
#endif
}

static void ReadRaw(timestamp_t *timestamp) {
  Lock();
  HAL_RTC_GetTime(RT.prtc, &timestamp->time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(RT.prtc, &timestamp->date, RTC_FORMAT_BIN);
  timestamp->tzQuarterHour = 0;
  UnLock();
}

static void WriteRaw(timestamp_t *timestamp) {
  // add extra property
  timestamp->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  timestamp->time.StoreOperation = RTC_STOREOPERATION_RESET;

  // set the RTC
  Lock();
  HAL_RTC_SetTime(RT.prtc, &timestamp->time, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(RT.prtc, &timestamp->date, RTC_FORMAT_BIN);
  UnLock();
}

static timestamp_t Decode(datetime_t dt) {
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

static datetime_t Encode(timestamp_t ts) {
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
