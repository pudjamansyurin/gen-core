/*
 * _ublox.c
 *
 *  Created on: Mar 4, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_gps.h"

#include "DMA/_dma_ublox.h"
#include "usart.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osThreadId_t GpsTaskHandle;
extern osMutexId_t GpsRecMutexHandle;
#endif

/* Public variables
 * --------------------------------------------*/
gps_t GPS = {
    .d = {0},
    .puart = &huart2,
    .pdma = &hdma_usart2_rx,
};

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
#if GPS_DEBUG
static void Debugger(char *ptr, size_t len);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t GPS_Init(void) {
  uint8_t ok;
  uint32_t tick;

  lock();
  printf("GPS:Init\n");

  nmea_init(&(GPS.d.nmea));
  MX_USART2_UART_Init();
  UBLOX_DMA_Start(GPS.puart, GPS.pdma, GPS_ReceiveCallback);
  GATE_GpsReset();

  tick = _GetTickMS();
  while (_GetTickMS() - tick < GPS_TIMEOUT_MS)
    if (GPS.d.tick) break;

  ok = GPS.d.tick > 0;
  unlock();

  printf("GPS:%s\n", ok ? "OK" : "Error");
  return ok;
}

void GPS_DeInit(void) {
  lock();
  GPS_Flush();
  GATE_GpsShutdown();
  UBLOX_DMA_Stop();
  HAL_UART_DeInit(GPS.puart);
  unlock();
}

void GPS_Refresh(void) {
  lock();
  GPS.d.active = _TickIn(GPS.d.tick, GPS_TIMEOUT_MS);
  if (!GPS.d.active) {
    GPS_DeInit();
    _DelayMS(500);
    GPS_Init();
  }
  unlock();
}

void GPS_Flush(void) {
  lock();
  memset(&(GPS.d), 0, sizeof(gps_data_t));
  unlock();
}

void GPS_ReceiveCallback(void *ptr, size_t len) {
#if GPS_DEBUG
  Debugger(ptr, len);
#endif
  if (nmea_process(&(GPS.d.nmea), (char *)ptr, len)) GPS.d.tick = _GetTickMS();
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(GpsRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(GpsRecMutexHandle);
#endif
}

#if GPS_DEBUG
static void Debugger(char *ptr, size_t len) { printf("\n%.*s\n", len, ptr); }
#endif
