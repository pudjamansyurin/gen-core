/*
 * _ublox.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef GPS_H_
#define GPS_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_nmea.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define GPS_INTERVAL          (uint16_t) 1				// in second

/* Exported struct -----------------------------------------------------------*/
typedef struct {
	nmea_float_t dop_h;
	nmea_float_t dop_v;
	nmea_float_t altitude;
	nmea_float_t latitude;
	nmea_float_t longitude;
	nmea_float_t heading;
	nmea_float_t speed_kph;
	nmea_float_t speed_mps;
  uint8_t sat_in_use;
  uint8_t fix;
} gps_t;

/* Public functions prototype ------------------------------------------------*/
void GPS_Init(void);
uint8_t GPS_Capture(gps_t *gps);
uint8_t GPS_CalculateOdometer(gps_t *gps);
uint8_t GPS_CalculateSpeed(gps_t *gps);
void GPS_Debugger(void);

#endif /* GPS_H_ */
