/*
 * _rtc.c
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_rtc.h"

/* External variables ----------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
extern osMutexId_t RTCMutexHandle;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
timestamp_t RTC_Decode(uint64_t dateTime) {
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
  RTC_ReadRaw(&timestamp);

  // encode timestamp to datetime
  return RTC_Encode(timestamp);
}

void RTC_ReadRaw(timestamp_t *timestamp) {
  // get the RTC
  lock();
  HAL_RTC_GetTime(&hrtc, &timestamp->time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &timestamp->date, RTC_FORMAT_BIN);
  unlock();
}

void RTC_Write(uint64_t dateTime, rtc_t *rtc) {
  timestamp_t timestamp;

  // decode datetime to timestamp
  timestamp = RTC_Decode(dateTime);

  // set the RTC
  RTC_WriteRaw(&timestamp, rtc);
}

void RTC_WriteRaw(timestamp_t *timestamp, rtc_t *rtc) {
  // add extra property
  timestamp->time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  timestamp->time.StoreOperation = RTC_STOREOPERATION_RESET;

  // set the RTC
  lock();
  HAL_RTC_SetTime(&hrtc, &timestamp->time, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, &timestamp->date, RTC_FORMAT_BIN);
  unlock();

  // save calibration date
  // source from server is always considered as valid
  rtc->calibration = timestamp->date;
  // update time
  rtc->timestamp = *timestamp;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  osMutexAcquire(RTCMutexHandle, osWaitForever);
}

static void unlock(void) {
  osMutexRelease(RTCMutexHandle);
}
