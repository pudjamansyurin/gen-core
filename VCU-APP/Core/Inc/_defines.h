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

#define VCU_VERSION (uint16_t)635
#define EEPROM_RESET (uint16_t)53

#define CAN_DEBUG		0
#define GPS_DEBUG		0
#define MEMS_DEBUG		0
#define REMOTE_DEBUG 	0
#define FINGER_DEBUG    1

#define AT_USE_CLK		1
#define AT_USE_SMS   	1
#define AT_USE_TCP	 	1
#define AT_USE_FTP	 	0

/* Exported constants --------------------------------------------------------*/
#define VCU_VENDOR "GEN"
#define VCU_BUILD_YEAR (uint8_t)21

#define MANAGER_WAKEUP (uint16_t) 1111 // in ms

#define MCU_SPEED_MAX (uint8_t)150 // in kph
#define BMS_LOWBAT (uint8_t)20

#define SIMCOM_VOLTAGE_MIN (uint16_t)3300 // in mV
#define HMI_FOTA_TIMEOUT (uint32_t)20000  // in ms

#define VCU_ACTIVATE_LOST (uint16_t)(5 * 60) // in second

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
