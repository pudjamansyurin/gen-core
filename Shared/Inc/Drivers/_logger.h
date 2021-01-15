/*
 * _logger.h
 *
 *  Created on: Jan 15, 2021
 *      Author: pudja
 */

#ifndef INC_LIBS__LOGGER_H_
#define INC_LIBS__LOGGER_H_

/* Includes ------------------------------------------------------------------*/
#include "_defines.h"

/* Public functions prototype ------------------------------------------------*/
void LogInit(void);
void Log(const char *fmt, ...);

#endif /* INC_LIBS__LOGGER_H_ */
