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

/* Exported variables
 * ----------------------------------------------------------*/
extern nmea_t GPS;

/* Exported constants --------------------------------------------------------*/
#define GPS_INTERVAL (uint16_t)1 // in second
#define GPS_LENGTH_MIN (uint8_t)100

/* Exported struct -----------------------------------------------------------*/
typedef struct {
  UART_HandleTypeDef *puart;
  DMA_HandleTypeDef *pdma;
} gps_t;

/* Public functions prototype ------------------------------------------------*/
void GPS_Init(void);
void GPS_DeInit(void);
void GPS_ReceiveCallback(void *ptr, size_t len);
uint8_t GPS_CalculateOdometer(void);

#endif /* GPS_H_ */
