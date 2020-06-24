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

/* Public functions prototype ------------------------------------------------*/
timestamp_t RTC_Decode(uint64_t dateTime);
uint64_t RTC_Encode(timestamp_t timestamp);
uint64_t RTC_Read(void);
void RTC_ReadRaw(timestamp_t *timestamp);
void RTC_Write(uint64_t dateTime, rtc_t *rtc);
void RTC_WriteRaw(timestamp_t *timestamp, rtc_t *rtc);
uint8_t RTC_IsDaylight(timestamp_t timestamp);
uint8_t RTC_NeedCalibration(void);
void RTC_CalibrateWithSimcom(void);

#endif /* RTC_H_ */
