/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_ublox.h"
#include "Libs/_gps.h"

/* Private variables ----------------------------------------------------------*/
static nmea_t nmea;
static UART_HandleTypeDef *uart;

/* Public functions implementation --------------------------------------------*/
void GPS_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
	uint32_t tick;

  uart = huart;
  HAL_UART_Init(huart);
  UBLOX_DMA_Init(huart, hdma);

	// Inititalize Module
	do {
		LOG_StrLn("GPS:Init");

		GATE_GpsReset();

		// set timeout guard
		tick = _GetTickMS();
		while ((_GetTickMS() - tick) < 5000) {
			if (strlen(UBLOX_UART_RX) > 50)
				break;
			_DelayMS(10);
		}
	} while (strlen(UBLOX_UART_RX) <= 50);

	nmea_init(&nmea);
}

void GPS_DeInit(void) {
  GATE_GpsShutdown();
  UBLOX_DMA_DeInit();
  HAL_UART_DeInit(uart);
}

uint8_t GPS_Capture(gps_t *gps) {
	nmea_process(&nmea, UBLOX_UART_RX, strlen(UBLOX_UART_RX));

	// copy only necessary part
	gps->dop_h = nmea.dop_h;
	gps->dop_v = nmea.dop_v;
	gps->altitude = nmea.altitude;
	gps->latitude = nmea.latitude;
	gps->longitude = nmea.longitude;
	gps->heading = nmea.coarse;
  gps->sat_in_use = nmea.sats_in_use;
	gps->speed_kph = nmea_to_speed(nmea.speed, nmea_speed_kph);
	gps->speed_mps = nmea_to_speed(nmea.speed, nmea_speed_mps);
  gps->fix = nmea.fix;

  return gps->fix > 0;
}

uint8_t GPS_CalculateOdometer(gps_t *gps) {
  if (gps->speed_mps)
    return (gps->speed_mps * GPS_INTERVAL );
  return 0;
}

uint8_t GPS_CalculateSpeed(gps_t *gps) {
  if (gps->speed_kph > 8)
    return gps->speed_kph + 8;
  return gps->speed_kph * 2;
}

void GPS_Debugger(void) {
  LOG_StrLn("GPS:Buffer = ");
  LOG_Buf(UBLOX_UART_RX, sizeof(UBLOX_UART_RX));
  LOG_Enter();
}
