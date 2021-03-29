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
#include "usart.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osThreadId_t GpsTaskHandle;
extern osMutexId_t GpsRecMutexHandle;
#endif

/* Public variables
 * ----------------------------------------------------------*/
gps_t GPS = {
		.d = {0},
		.puart = &huart2,
		.pdma = &hdma_usart2_rx,
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
#if GPS_DEBUG
static void Debugger(char *ptr, size_t len);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t GPS_Init(void) {
	lock();

	printf("GPS:Init\n");
	MX_USART2_UART_Init();
	UBLOX_DMA_Start(GPS.puart, GPS.pdma, GPS_ReceiveCallback);
	GATE_GpsReset();
	_DelayMS(GPS_TIMEOUT);

	nmea_init(&(GPS.d.nmea));
	unlock();

	return GPS.d.tick > 0;
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
	GPS.d.active = GPS.d.tick && (_GetTickMS() - GPS.d.tick) < GPS_TIMEOUT;
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
	if (nmea_process(&(GPS.d.nmea), (char *)ptr, len))  {
		GPS.d.tick = _GetTickMS();
		osThreadFlagsSet(GpsTaskHandle, FLAG_GPS_RECEIVED);
	}
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(GpsRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
	osMutexRelease(GpsRecMutexHandle);
#endif
}

#if GPS_DEBUG
static void Debugger(char *ptr, size_t len) {
	printf("\n%.*s\n", len, ptr);
}
#endif
