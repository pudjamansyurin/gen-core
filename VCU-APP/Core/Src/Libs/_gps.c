/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_gps.h"
#include "DMA/_dma_ublox.h"
#include "Nodes/VCU.h"

/* External variables ---------------------------------------------------------*/
extern char UBLOX_UART_RX[UBLOX_UART_RX_SZ ];
extern vcu_t VCU;

/* Private variables -----------------------------------------------------------*/
gps_t GPS;

/* Private variables ----------------------------------------------------------*/
static nmea_t nmea;

/* Public functions implementation --------------------------------------------*/
void GPS_Init(void) {
	uint32_t tick;

	// Inititalize Module
	do {
		LOG_StrLn("GPS:Init");

		HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, 0);
		_DelayMS(500);
		HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, 1);

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

uint8_t GPS_Capture(void) {
	nmea_process(&nmea, UBLOX_UART_RX, strlen(UBLOX_UART_RX));

	// copy only necessary part
	GPS.dop_h = nmea.dop_h;
	GPS.dop_v = nmea.dop_v;
	GPS.altitude = nmea.altitude;
	GPS.latitude = nmea.latitude;
	GPS.longitude = nmea.longitude;
	GPS.heading = nmea.coarse;
	GPS.speed_kph = nmea_to_speed(nmea.speed, nmea_speed_kph);
	GPS.speed_mps = nmea_to_speed(nmea.speed, nmea_speed_mps);
  GPS.sat_in_use = nmea.sats_in_use;
  GPS.fix = nmea.fix;

  return GPS.fix > 0;
}

void GPS_CalculateOdometer(void) {
  uint8_t increment;

  // calculate
  if (GPS.speed_mps > 5) {
    increment = (GPS.speed_mps * GPS_INTERVAL );

    VCU.SetOdometer(increment);
    HBAR_AccumulateSubTrip(increment);
  }
}

void GPS_CalculateSpeed(void) {
  // FIXME: use real data
  VCU.d.speed = GPS.speed_kph;
  VCU.d.volume = VCU.d.speed * 100 / MCU_SPEED_KPH_MAX;
}

void GPS_Debugger(void) {
  LOG_StrLn("GPS:Buffer = ");
  LOG_Buf(UBLOX_UART_RX, sizeof(UBLOX_UART_RX));
  LOG_Enter();
}
