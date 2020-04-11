/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_gps.h"

/* External variables ---------------------------------------------------------*/
extern char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static nmea_t nmea;

/* Public functions implementation --------------------------------------------*/
void GPS_Init(void) {
  // mosfet control
  while (1) {
    LOG_StrLn("GPS:Init");

    HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, 0);
    _LedWrite(1);
    osDelay(500);
    HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, 1);
    _LedWrite(0);
    osDelay(2000);

    // debug
    if (strlen(UBLOX_UART_RX) > 50) {
      break;
    }
  };
  _LedWrite(1);

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
