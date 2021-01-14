/*
 * _rtc.h
 *
 *  Created on: Oct 9, 2019
 *      Author: Puja
 */

#ifndef RTC_H_
#define RTC_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Typedef -------------------------------------------------------------------*/
typedef struct {
  timestamp_t timestamp;
  RTC_DateTypeDef calibration;
} datetime_t;

typedef struct {
  struct {
    RTC_HandleTypeDef *rtc;
  } h;
} rtc_t;

/* Public functions prototype ------------------------------------------------*/
void RTC_Init(RTC_HandleTypeDef *hrtc);
timestamp_t RTC_Decode(uint64_t dateTime);
uint64_t RTC_Encode(timestamp_t timestamp);
uint64_t RTC_Read(void);
void RTC_ReadRaw(timestamp_t *timestamp);
void RTC_Write(uint64_t dateTime, datetime_t *dt);
void RTC_WriteRaw(timestamp_t *timestamp, datetime_t *dt);
uint8_t RTC_IsDaylight(timestamp_t timestamp);
uint8_t RTC_NeedCalibration(datetime_t *dt);
void RTC_CalibrateWithSimcom(datetime_t *dt);

#endif /* RTC_H_ */
