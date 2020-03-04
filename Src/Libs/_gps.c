/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#include "_gps.h"

extern char UBLOX_UART_RX_Buffer[UBLOX_UART_RX_BUFFER_SIZE];
nmea_t hnmea;

void GPS_Init(void) {
	HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_SET);
	osDelay(100);

	nmea_init(&hnmea);
}

uint8_t GPS_Process(gps_t *hgps) {
	nmea_process(&hnmea, UBLOX_UART_RX_Buffer, strlen(UBLOX_UART_RX_Buffer));

	// copy only necessary part
	hgps->dop_h = hnmea.dop_h;
	hgps->latitude = hnmea.latitude;
	hgps->longitude = hnmea.longitude;
	hgps->heading = hnmea.variation;
	hgps->speed_kph = nmea_to_speed(hnmea.speed, nmea_speed_kph);
	hgps->speed_mps = nmea_to_speed(hnmea.speed, nmea_speed_mps);

	return hnmea.fix;
}
