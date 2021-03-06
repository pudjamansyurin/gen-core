/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "DMA/_dma_ublox.h"
#include "Libs/_gps.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osThreadId_t GpsTaskHandle;
#endif

/* Private variables ----------------------------------------------------------*/
static gps_t gps;

/* Private functions protoypes -----------------------------------------------*/
static void Debugger(char *ptr, size_t len);

/* Public functions implementation --------------------------------------------*/
void GPS_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
  uint32_t tick;

  gps.h.uart = huart;
  gps.h.dma = hdma;

  MX_USART2_UART_Init();
  UBLOX_DMA_Start(huart, hdma, GPS_ProcessBuffer);

  // Initiate Module
  do {
    printf("GPS:Init\n");

    GATE_GpsReset();

    // set timeout guard
    tick = _GetTickMS();
    while ((_GetTickMS() - tick) < 5000) {
      if (strnlen(UBLOX_UART_RX, UBLOX_UART_RX_SZ) > 50)
        break;
      _DelayMS(10);
    }
  } while (strnlen(UBLOX_UART_RX, UBLOX_UART_RX_SZ) <= 50);

  nmea_init(&(gps.nmea));
}

void GPS_DeInit(void) {
  GATE_GpsShutdown();
  UBLOX_DMA_Stop();
  HAL_UART_DeInit(gps.h.uart);
}

void GPS_ProcessBuffer(void *ptr, size_t len) {
  nmea_process(&(gps.nmea), (char*) ptr, len);
  //  Debugger(ptr, len);
  osThreadFlagsSet(GpsTaskHandle, EVT_GPS_RECEIVED);
}

uint8_t GPS_Capture(gps_data_t *data) {
  // copy only necessary part
  data->dop_h = gps.nmea.dop_h;
  data->dop_v = gps.nmea.dop_v;
  data->altitude = gps.nmea.altitude;
  data->latitude = gps.nmea.latitude;
  data->longitude = gps.nmea.longitude;
  data->heading = gps.nmea.coarse;
  data->sat_in_use = gps.nmea.sats_in_use;
  data->speed_kph = nmea_to_speed(gps.nmea.speed, nmea_speed_kph);
  data->speed_mps = nmea_to_speed(gps.nmea.speed, nmea_speed_mps);
  data->fix = gps.nmea.fix;

  return data->fix > 0;
}

uint8_t GPS_CalculateOdometer(gps_data_t *data) {
  if (data->speed_mps)
    return (data->speed_mps * GPS_INTERVAL );
  return 0;
}

uint8_t GPS_CalculateSpeed(gps_data_t *data) {
  if (data->speed_kph > 8)
    return data->speed_kph + 8;
  return data->speed_kph * 2;
}

/* Private functions implementation --------------------------------------------*/
static void Debugger(char *ptr, size_t len) {
  printf("\n%.*s\n", len, ptr);
}
