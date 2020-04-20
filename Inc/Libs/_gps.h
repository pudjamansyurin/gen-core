/*
 * _ublox.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef GPS_H_
#define GPS_H_

/* Includes ------------------------------------------------------------------*/
#include "_nmea.h"

/* Exported constants --------------------------------------------------------*/
#define GPS_INTERVAL_MS                 2500

/* Exported struct --------------------------------------------------------------*/
typedef struct {
  nmea_float_t dop_h;
  nmea_float_t latitude;
  nmea_float_t longitude;
  nmea_float_t heading;
  nmea_float_t speed_kph;
  nmea_float_t speed_mps;
} gps_t;

/* Public functions prototype ------------------------------------------------*/
void GPS_Init(void);
uint8_t GPS_Capture(void);
void GPS_CalculateOdometer(void);

#endif /* GPS_H_ */
