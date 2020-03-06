/*
 * _ublox.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef GPS_H_
#define GPS_H_

#include "_nmea.h"

typedef struct {
  nmea_float_t dop_h;
  nmea_float_t latitude;
  nmea_float_t longitude;
  nmea_float_t heading;
  nmea_float_t speed_kph;
  nmea_float_t speed_mps;
} gps_t;

/* Public functions ---------------------------------------------------------*/
void GPS_Init(void);
uint8_t GPS_Process(gps_t *hgps);

#endif /* GPS_H_ */
