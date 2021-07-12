/*
 * _ublox.h
 *
 *  Created on: Mar 4, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__GPS_H_
#define INC_LIBS__GPS_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_nmea.h"

/* Exported types
 * --------------------------------------------*/
typedef struct {
  uint8_t active;
  uint32_t tick;
} gps_data_t;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t GPS_Init(void);
void GPS_DeInit(void);
void GPS_Refresh(void);
void GPS_Flush(void);
void GPS_ReceiveCallback(const void* ptr, size_t len);

const gps_data_t* GPS_IO_Data(void);
const nmea_t *GPS_IO_Nmea(void);
#endif /* INC_LIBS__GPS_H_ */
