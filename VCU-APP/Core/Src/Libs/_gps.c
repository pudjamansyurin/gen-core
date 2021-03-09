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
extern osMutexId_t GpsMutexHandle;
#endif

/* Private variables ----------------------------------------------------------*/
static gps_t gps = {
		.puart = &huart2,
		.pdma = &hdma_usart2_rx,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t DataAvailable(void);
static void Debugger(char *ptr, size_t len);

/* Public functions implementation --------------------------------------------*/
void GPS_Init(void) {
  uint32_t tick;

  lock();
  MX_USART2_UART_Init();
  UBLOX_DMA_Start(gps.puart, gps.pdma, GPS_ProcessBuffer);

  // Initiate Module
  do {
    printf("GPS:Init\n");

    GATE_GpsReset();

    // set timeout guard
    tick = _GetTickMS();
    while ((_GetTickMS() - tick) < 5000) {
      if (DataAvailable())
        break;
      _DelayMS(10);
    }
  } while (!DataAvailable());

  nmea_init(&(gps.nmea));
  unlock();
}

void GPS_DeInit(void) {
	lock();
  GATE_GpsShutdown();
  UBLOX_DMA_Stop();
  HAL_UART_DeInit(gps.puart);
  unlock();
}

void GPS_ProcessBuffer(void *ptr, size_t len) {
	if (!DataAvailable()) return;

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
static void lock(void) {
  #if (RTOS_ENABLE)
  osMutexAcquire(GpsMutexHandle, osWaitForever);
  #endif
}

static void unlock(void) {
  #if (RTOS_ENABLE)
  osMutexRelease(GpsMutexHandle);
  #endif
}

static uint8_t DataAvailable(void) {
	const uint8_t minDataInBuffer = 50;

	return strnlen(UBLOX_UART_RX, UBLOX_UART_RX_SZ) > minDataInBuffer;
}

static void Debugger(char *ptr, size_t len) {
  printf("\n%.*s\n", len, ptr);
}
