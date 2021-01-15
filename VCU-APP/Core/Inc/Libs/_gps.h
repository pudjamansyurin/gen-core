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
  struct {
    UART_HandleTypeDef *uart;
    DMA_HandleTypeDef *dma;
  } h;
} gps_t;

/* Public functions prototype ------------------------------------------------*/
void GPS_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
uint8_t GPS_Capture(gps_data_t *data);
uint8_t GPS_CalculateOdometer(gps_data_t *data);
uint8_t GPS_CalculateSpeed(gps_data_t *data);

#endif /* GPS_H_ */
