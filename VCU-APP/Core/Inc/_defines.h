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

#define VCU_VERSION (uint16_t)613
#define EEPROM_RESET (uint16_t)53

/* Exported constants --------------------------------------------------------*/
#define VCU_VENDOR "GEN"
#define VCU_BUILD_YEAR (uint8_t)21

#define MANAGER_WAKEUP (uint16_t) 111 // in ms

#define MCU_SPEED_MAX (uint8_t)150 // in kph
#define MCU_RPM_MAX (uint32_t)99999
#define ODOMETER_MAX (uint32_t)99999 // in km
#define BMS_LOWBAT (uint8_t)20

#define SIMCOM_VOLTAGE_MIN (uint16_t)3300 // in mV
#define HMI_FOTA_TIMEOUT (uint32_t)20000  // in ms

#define REMOTE_TIMEOUT (uint32_t)7000         // in ms
#define REMOTE_TIMEOUT_RUN (uint32_t)30000    // in ms
#define REMOTE_PAIRING_TIMEOUT (uint32_t)5000 // in ms

#define FINGER_CONFIDENCE_MIN (uint8_t)75  // in %
#define FINGER_SCAN_TIMEOUT (uint32_t)5000 // in ms
#define FINGER_USER_MAX (uint8_t)5
#define DRIVER_ID_NONE (uint8_t)0xFF

#define RPT_FRAME_FULL (uint16_t)20      // in second
#define RPT_INTERVAL_NORMAL (uint16_t)5  // in second
#define RPT_INTERVAL_BACKUP (uint16_t)20 // in second
#define RPT_INTERVAL_LOST (uint16_t)60   // in second

#define VCU_ACTIVATE_LOST (uint16_t)(5 * 60) // in second

// Events (for Individual Thread)
#define EVT_MASK (uint32_t)0x7FFFFFFF

#define EVT_IOT_REPORT_DISCARD BIT(0)
#define EVT_IOT_RESUBSCRIBE BIT(1)
#define EVT_IOT_SEND_USSD BIT(2)
#define EVT_IOT_READ_SMS BIT(3)

#define EVT_REPORTER_YIELD BIT(0)

#define EVT_COMMAND_ERROR BIT(0)
#define EVT_COMMAND_OK BIT(1)

#define EVT_GPS_TASK_START BIT(0)
#define EVT_GPS_TASK_STOP BIT(1)
#define EVT_GPS_RECEIVED BIT(2)

#define EVT_GYRO_TASK_START BIT(0)
#define EVT_GYRO_TASK_STOP BIT(1)
#define EVT_GYRO_MOVED_RESET BIT(2)

#define EVT_REMOTE_TASK_START BIT(0)
#define EVT_REMOTE_TASK_STOP BIT(1)
#define EVT_REMOTE_REINIT BIT(2)
#define EVT_REMOTE_RX_IT BIT(3)
#define EVT_REMOTE_PAIRING BIT(4)

#define EVT_FINGER_TASK_START BIT(0)
#define EVT_FINGER_TASK_STOP BIT(1)
#define EVT_FINGER_PLACED BIT(2)
#define EVT_FINGER_FETCH BIT(3)
#define EVT_FINGER_ADD BIT(4)
#define EVT_FINGER_DEL BIT(5)
#define EVT_FINGER_RST BIT(6)

#define EVT_AUDIO_TASK_START BIT(0)
#define EVT_AUDIO_TASK_STOP BIT(1)
#define EVT_AUDIO_BEEP BIT(2)
#define EVT_AUDIO_BEEP_START BIT(3)
#define EVT_AUDIO_BEEP_STOP BIT(4)
#define EVT_AUDIO_MUTE_ON BIT(5)
#define EVT_AUDIO_MUTE_OFF BIT(6)

#define EVT_CAN_TASK_START BIT(0)
#define EVT_CAN_TASK_STOP BIT(1)

#define EVT_GATE_HBAR BIT(0)
#define EVT_GATE_STARTER_IRQ BIT(1)

#define EVT_HMI2POWER_CHANGED BIT(0)

// Events group (for All Threads)
#define EVENT_MASK (uint32_t)0xFFFFFF
#define EVENT_READY BIT(0)

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
  PAYLOAD_MAX = 1,
} PAYLOAD_TYPE;

#endif /* DEFINES_H_ */
