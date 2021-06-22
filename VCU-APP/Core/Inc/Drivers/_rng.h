/*
 * _rng.h
 *
 *  Created on: Mar 8, 2021
 *      Author: pudja
 */

#ifndef INC_DRIVERS__RNG_H_
#define INC_DRIVERS__RNG_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t RNG_Generate32(uint32_t *payload, uint8_t size);

#endif /* INC_DRIVERS__RNG_H_ */
