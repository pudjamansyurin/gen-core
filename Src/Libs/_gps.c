/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#include "_gps.h"

extern char UBLOX_UART_RX[UBLOX_UART_RX_SZ];
static nmea_t nmea;

void GPS_Init(void) {
	HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_SET);
	osDelay(100);

	nmea_init(&nmea);
}

uint8_t GPS_Process(gps_t *hgps) {
	nmea_process(&nmea, UBLOX_UART_RX, strlen(UBLOX_UART_RX));

	// copy only necessary part
	hgps->dop_h = nmea.dop_h;
	hgps->latitude = nmea.latitude;
	hgps->longitude = nmea.longitude;
	hgps->heading = nmea.variation;
	hgps->speed_kph = nmea_to_speed(nmea.speed, nmea_speed_kph);
	hgps->speed_mps = nmea_to_speed(nmea.speed, nmea_speed_mps);

	return nmea.fix;
}
