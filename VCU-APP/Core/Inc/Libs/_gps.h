/*
 * _ublox.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef GPS_H_
#define GPS_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_nmea.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define GPS_INTERVAL          (uint16_t) 1				// in second
#define GPS_MIN_LENGTH				 (uint8_t) 100

/* Exported struct -----------------------------------------------------------*/
typedef struct {
	float dop_h;
	float dop_v;
	float altitude;
	float latitude;
	float longitude;
	float heading;
	float speed_kph;
	float speed_mps;
	uint8_t sat_in_use;
	uint8_t fix;
} gps_data_t;

typedef struct {
	nmea_t nmea;
	UART_HandleTypeDef *puart;
	DMA_HandleTypeDef *pdma;
} gps_t;

/* Public functions prototype ------------------------------------------------*/
void GPS_Init(void);
void GPS_ProcessBuffer(void *ptr, size_t len);
uint8_t GPS_Capture(void);
uint8_t GPS_CalculateOdometer(void);

#endif /* GPS_H_ */
