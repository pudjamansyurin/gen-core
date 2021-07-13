/*
 * printf.h
 *
 *  Created on: Jan 15, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__PRINTF_H_
#define INC_DRIVERS__PRINTF_H_

/* Includes
 * --------------------------------------------*/
#include "defs.h"

/* Public functions prototype
 * --------------------------------------------*/
void printf_init(void);
void printf_hex(const char* data, uint16_t size);

#endif /* INC_DRIVERS__PRINTF_H_ */
