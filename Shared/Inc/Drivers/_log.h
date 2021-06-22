/*
 * _log.h
 *
 *  Created on: Jan 15, 2021
 *      Author: pudja
 */

#ifndef INC_LIBS__LOGGER_H_
#define INC_LIBS__LOGGER_H_

/* Includes
 * --------------------------------------------*/
#include "_defines.h"

/* Exported constants
 * --------------------------------------------*/
#define LOG_TIMEOUT_MS ((uint16_t)2)

/* Public functions prototype
 * --------------------------------------------*/
void printf_init(void);
void printf_hex(char *data, uint16_t size);
// void Log(const char *fmt, ...);

#endif /* INC_LIBS__LOGGER_H_ */
