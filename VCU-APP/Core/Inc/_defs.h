/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/* Choose  between APP or BL mode */
#define APP 1

/* Includes
 * --------------------------------------------*/
#include "_defs_shared.h"

/* Exported constants
 * --------------------------------------------*/
#define VCU_VERSION ((uint16_t)662)

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

#define SIM_MIN_MV ((uint16_t)3300)

#define VCU_VENDOR "GEN"
#define VCU_BUILD_YEAR ((uint8_t)21)
#define HMI_FOTA_MS ((uint32_t)20000)
#define VCU_LOST_MODE_S ((uint16_t)(5 * 60))

//#define MCU_SPEED_MAX_KPH ((uint8_t)140)
//#define MCU_DISCUR_MAX ((uint8_t)210)

#endif /* DEFINES_H_ */