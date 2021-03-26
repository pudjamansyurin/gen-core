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
static uint8_t DataInvalid(void);
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

	if (!DataInvalid())
		nmea_init(&(GPS.d.nmea));
	unlock();

	return DataInvalid();
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
	if (DataInvalid()) return;

#if GPS_DEBUG
	Debugger(ptr, len);
#endif
	GPS.d.tick = _GetTickMS();
	nmea_process(&(GPS.d.nmea), (char *)ptr, len);
	osThreadFlagsSet(GpsTaskHandle, FLAG_GPS_RECEIVED);
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

static uint8_t DataInvalid(void) {
	return strnlen(UBLOX_UART_RX, UBLOX_UART_RX_SZ) < GPS_LENGTH_MIN;
}

#if GPS_DEBUG
static void Debugger(char *ptr, size_t len) {
	printf("\n%.*s\n", len, ptr);
}
#endif
