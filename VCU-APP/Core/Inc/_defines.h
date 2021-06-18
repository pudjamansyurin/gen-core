/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/* Choose  between VCU or Boot-loader mode */
#define BOOTLOADER 0

/* Includes ------------------------------------------------------------------*/
#include "_defines_shared.h"

#define VCU_VERSION ((uint16_t)657)
//#define EE_RESET ((uint16_t)53)

#define SIMCOM_DEBUG  	1
#define CAN_DEBUG		0
#define GPS_DEBUG		0
#define MEMS_DEBUG		0
#define REMOTE_DEBUG 	0
#define FINGER_DEBUG  	0

#define AT_USE_CLK		1
#define AT_USE_SMS   	1
#define AT_USE_TCP	 	1
#define AT_USE_FTP	 	0

#define SIMCOM_MIN_MV ((uint16_t)3300)

/* Exported constants --------------------------------------------------------*/
#define VCU_VENDOR "GEN"
#define VCU_BUILD_YEAR ((uint8_t)21)
#define MANAGER_WAKEUP_MS ((uint16_t)555)
#define HMI_FOTA_MS ((uint32_t)20000)
#define VCU_LOST_MODE_S ((uint16_t)(5 * 60))

//#define MCU_SPEED_MAX_KPH ((uint8_t)140)
//#define MCU_DISCUR_MAX ((uint8_t)210)

/* Exported typedef ----------------------------------------------------------*/
typedef enum {
  VEHICLE_UNKNOWN = -3,
  VEHICLE_LOST = -2,
  VEHICLE_BACKUP = -1,
  VEHICLE_NORMAL = 0,
  VEHICLE_STANDBY = 1,
  VEHICLE_READY = 2,
  VEHICLE_RUN = 3,
} vehicle_state_t;

typedef enum {
  PAYLOAD_RESPONSE = 0,
  PAYLOAD_REPORT,
  PAYLOAD_MAX = 2,
} PAYLOAD_TYPE;

#endif /* DEFINES_H_ */
