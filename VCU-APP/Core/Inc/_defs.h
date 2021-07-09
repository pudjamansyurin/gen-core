/*
 * _defs.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef DEFS_H_
#define DEFS_H_

/* Choose  between APP or BL mode */
#define APP 1

/* Includes
 * --------------------------------------------*/
#include "_defs_shared.h"

/* Exported constants
 * --------------------------------------------*/
#define VCU_VERSION ((uint16_t)662)
#define VCU_BUILD_YEAR ((uint8_t)21)
#define VCU_VENDOR "GEN"

#define SIM_DEBUG 1
#define CAN_DEBUG 1
#define GPS_DEBUG 1
#define MEMS_DEBUG 1
#define REMOTE_DEBUG 1
#define FINGER_DEBUG 1

#define AT_USE_CLK 1
#define AT_USE_SMS 1
#define AT_USE_TCP 1
#define AT_USE_FTP 0

#define SIM_BAT_MIN_MV ((uint16_t)3300)

#endif /* DEFS_H_ */
