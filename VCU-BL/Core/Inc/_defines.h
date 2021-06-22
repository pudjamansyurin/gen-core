/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/* Choose between VCU or Boot-loader mode */
#define BOOTLOADER 1

/* Includes
 * --------------------------------------------*/
#include "_defines_shared.h"

/* Exported constants
 * --------------------------------------------*/
#define SIMCOM_DEBUG 1
#define CAN_DEBUG 0

#define AT_USE_CLK 0
#define AT_USE_SMS 0
#define AT_USE_TCP 0
#define AT_USE_FTP 1

#define SIMCOM_MIN_MV ((uint16_t)3400)

#endif /* DEFINES_H_ */
