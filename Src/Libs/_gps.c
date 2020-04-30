/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_gps.h"
#include "_reporter.h"
#include "_handlebar.h"

/* External variables ---------------------------------------------------------*/
extern char UBLOX_UART_RX[UBLOX_UART_RX_SZ];
extern db_t DB;

/* Public variables -----------------------------------------------------------*/
gps_t GPS;

/* Private variables ----------------------------------------------------------*/
static nmea_t nmea;

/* Public functions implementation --------------------------------------------*/
void GPS_Init(void) {
	uint32_t tick;

	// mosfet control
	do {
		LOG_StrLn("GPS:Init");

		HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, 0);
		_LedWrite(1);
		osDelay(500);
		HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, 1);
		_LedWrite(0);

		// set timeout guard
		tick = osKernelGetTickCount();
		while ((osKernelGetTickCount() - tick) < pdMS_TO_TICKS(5000)) {
			if (strlen(UBLOX_UART_RX) > 50) {
				break;
			}
			osDelay(10);
		}
	} while (strlen(UBLOX_UART_RX) <= 50);

	_LedWrite(0);

	nmea_init(&nmea);
}

uint8_t GPS_Capture(void) {
	nmea_process(&nmea, UBLOX_UART_RX, strlen(UBLOX_UART_RX));

	// copy only necessary part
	GPS.dop_h = nmea.dop_h;
	GPS.latitude = nmea.latitude;
	GPS.longitude = nmea.longitude;
	GPS.heading = nmea.coarse;
	GPS.speed_kph = nmea_to_speed(nmea.speed, nmea_speed_kph);
	GPS.speed_mps = nmea_to_speed(nmea.speed, nmea_speed_mps);

	return nmea.fix > 0;
}

void GPS_CalculateOdometer(void) {
	static uint16_t odometer_mps = 0;

	// dummy odometer
	if (GPS.speed_mps > 5) {
		odometer_mps += (GPS.speed_mps * GPS_INTERVAL_MS) / 1000;
	}
	// check if already > 1km
	if (odometer_mps >= 1000) {
		odometer_mps = 0;

		// Accumulate Odometer (Save permanently)
		EEPROM_Odometer(EE_CMD_W, DB.vcu.odometer + 1);

		// Accumulate Sub-Trip (Reset on Start)
		HBAR_AccumulateSubTrip();
	}

	// FIXME: use real data
	// update data
	DB.vcu.speed = GPS.speed_kph;
	DB.vcu.volume = DB.vcu.speed * 100 / MCU_SPEED_MAX;
}
