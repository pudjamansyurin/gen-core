/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DEFINES_SHARED_H_
#define DEFINES_SHARED_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#if (!BOOTLOADER)
#include "cmsis_os.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Exported macro functions --------------------------------------------------*/
#define BIT(x)                                  (1ULL << x)
#define BV(var, x)                              (var |= (1ULL << x))
#define BC(var, x)                              (var &= ~(1ULL << x))
#define BT(var, x)                              (var ^= (1ULL << x))
//#define _L(var, x)                              (var << x)
//#define _R(var, x)                              (var >> x)
#define _R1(var, x)                             ((var >> x) & 0x01)
#define _R2(var, x)                             ((var >> x) & 0x03)
#define _R8(var, x)                             ((var >> x) & 0xFF)

// FOTA related stuffs
#define SRAM_SIZE                    (uint32_t) 0x50000
#define SRAM_BASE_ADDR               (uint32_t) 0x20000000
#define SRAM_END_ADDR                           (SRAM_BASE_ADDR + SRAM_SIZE)
#define SP_MASK                      (uint32_t) 0x2FFFFFFF
#define SP_RANGE                                (SP_MASK - (SRAM_SIZE - 1))
#define DFU_PROGRESS_FLAG            (uint32_t) 0x89ABCDEF
#define IAP_FLAG                     (uint32_t) 0xAABBCCDD
#define IAP_FLAG_ADDR                           (SRAM_END_ADDR - sizeof(uint32_t))
#define IAP_RESPONSE_ADDR                       (IAP_FLAG_ADDR - sizeof(uint32_t))

#define IS_VALID_SP(a)                          ((*(__IO uint32_t*)a & SP_RANGE) == SRAM_BASE_ADDR)

/* Exported constants --------------------------------------------------------*/
#define RTOS_ENABLE                             !BOOTLOADER
#define EEPROM_RESET                 (uint16_t) 53

#define NET_BOOT_TIMEOUT             (uint32_t) 8000                // in ms
#define NET_EXTRA_TIME               (uint32_t) 1000                // in ms

#define NET_CON_APN                             "3gprs"
#define NET_CON_USERNAME                        "3gprs"
#define NET_CON_PASSWORD                        "3gprs"
//#define NET_CON_APN                             "telkomsel"
//#define NET_CON_USERNAME                        "wap"
//#define NET_CON_PASSWORD                        "wap123"
//#define NET_CON_APN                             "indosatgprs"
//#define NET_CON_USERNAME                        "indosat"
//#define NET_CON_PASSWORD                        "indosat"

#define NET_FTP_SERVER                          "ftp.genmotorcycles.com"
#define NET_FTP_USERNAME                        "fota@genmotorcycles.com"
#define NET_FTP_PASSWORD                        "@Garda313"

#if (!BOOTLOADER)
#define VCU_VENDOR                              "GEN"
#define VCU_BUILD_YEAR               (uint8_t)  20
#define VCU_UNITID                   (uint32_t) 354313

#define FOTA_MIN_VOLTAGE             (uint16_t) 3800

#define NET_TCP_SERVER                          "pujakusumae-30856.portmap.io"
#define NET_TCP_PORT                 (uint16_t) 46606

#define COMMAND_TIMEOUT              (uint32_t) 10000               // in ms

#define REMOTE_TIMEOUT               (uint32_t) 5000                // in ms
#define REMOTE_PAIRING_TIMEOUT       (uint32_t) 5000                 // in ms

#define PREFIX_REPORT                           "@R"
#define PREFIX_COMMAND                          "@C"
#define PREFIX_ACK                              "@A"
#define PREFIX_NACK                             "@N"

#define FINGER_CONFIDENCE_MIN        (uint8_t)  50
#define FINGER_SCAN_TIMEOUT          (uint32_t) 5000               // in ms
#define FINGER_USER_MAX              (uint8_t)  5

#define DRIVER_ID_NONE               (uint8_t)  0xFF

#define RPT_FRAME_FULL               (uint16_t) 20                  // in second
#define RPT_INTERVAL_NORMAL          (uint16_t) 5                  // in second
#define RPT_INTERVAL_BACKUP          (uint16_t) 20                 // in second
#define RPT_INTERVAL_LOST            (uint16_t) 60                 // in second

#define VCU_ACTIVATE_LOST            (uint16_t) (5*60)             // in second

// Payload list (Remote)
#define REMOTE_MSG_BROADCAST                   BIT(0)
#define REMOTE_MSG_FINDER                      BIT(1)
#define REMOTE_MSG_SEAT                        BIT(2)

// Events group (for Frame Report)
#define EV_VCU_NET_SOFT_RESET                   BIT(0)
#define EV_VCU_NET_HARD_RESET                   BIT(1)
#define EV_VCU_BIKE_FALLEN                      BIT(2)
#define EV_VCU_REMOTE_MISSING                   BIT(3)
#define EV_BMS_DISCHARGE_OVER_CURRENT           BIT(30)
#define EV_BMS_CHARGE_OVER_CURRENT              BIT(31)
#define EV_BMS_SHORT_CIRCUIT                    BIT(32)
#define EV_BMS_DISCHARGE_OVER_TEMPERATURE       BIT(33)
#define EV_BMS_DISCHARGE_UNDER_TEMPERATURE      BIT(34)
#define EV_BMS_CHARGE_OVER_TEMPERATURE          BIT(35)
#define EV_BMS_CHARGE_UNDER_TEMPERATURE         BIT(36)
#define EV_BMS_UNDER_VOLTAGE                    BIT(37)
#define EV_BMS_OVER_VOLTAGE                     BIT(38)
#define EV_BMS_OVER_DISCHARGE_CAPACITY          BIT(39)
#define EV_BMS_UNBALANCE                        BIT(40)
#define EV_BMS_SYSTEM_FAILURE                   BIT(41)
#define EV_BMS_WARNING_OVER_CURRENT             BIT(42)
#define EV_BMS_WARNING_OVER_TEMPERATURE         BIT(43)
#define EV_BMS_WARNING_UNDER_VOLTAGE            BIT(44)
#define EV_BMS_WARNING_UNBALANCE                BIT(45)

// Events (for Individual Thread)
#define EVT_MASK                     (uint32_t) 0x7FFFFFFF

#define EVT_IOT_DISCARD                         BIT(0)

#define EVT_COMMAND_ERROR                       BIT(0)
#define EVT_COMMAND_OK                          BIT(1)

#define EVT_GPS_TASK_START                      BIT(0)
#define EVT_GPS_TASK_STOP                       BIT(1)

#define EVT_GYRO_TASK_START                     BIT(0)
#define EVT_GYRO_TASK_STOP                      BIT(1)

#define EVT_REMOTE_TASK_START                   BIT(0)
#define EVT_REMOTE_TASK_STOP                    BIT(1)
#define EVT_REMOTE_RX_IT                        BIT(2)
#define EVT_REMOTE_PAIRING                      BIT(3)

#define EVT_FINGER_TASK_START                   BIT(0)
#define EVT_FINGER_TASK_STOP                    BIT(1)
#define EVT_FINGER_PLACED                       BIT(2)
#define EVT_FINGER_ADD                          BIT(3)
#define EVT_FINGER_DEL                          BIT(4)
#define EVT_FINGER_RST                          BIT(5)

#define EVT_AUDIO_TASK_START                    BIT(0)
#define EVT_AUDIO_TASK_STOP                     BIT(1)
#define EVT_AUDIO_BEEP                          BIT(2)
#define EVT_AUDIO_BEEP_START                    BIT(3)
#define EVT_AUDIO_BEEP_STOP                     BIT(4)
#define EVT_AUDIO_MUTE_ON                       BIT(5)
#define EVT_AUDIO_MUTE_OFF                      BIT(6)

#define EVT_HMI2POWER_CHANGED                   BIT(0)

#define EVT_GATE_HBAR                           BIT(0)
#define EVT_GATE_REG_5V_IRQ                     BIT(1)
#define EVT_GATE_STARTER_IRQ                    BIT(2)
#define EVT_GATE_KNOB_IRQ                       BIT(3)

// Events group (for All Threads)
#define EVENT_MASK                   (uint32_t) 0xFFFFFF
#define EVENT_READY                             BIT(0)

// Command Code List
#define CMD_CODE_GEN                  (uint8_t) 0
#define CMD_CODE_REPORT               (uint8_t) 1
#define CMD_CODE_AUDIO                (uint8_t) 2
#define CMD_CODE_FINGER               (uint8_t) 3
#define CMD_CODE_REMOTE              (uint8_t) 4

// Command Sub-Code List
#define CMD_GEN_INFO                  (uint8_t) 0
#define CMD_GEN_LED                   (uint8_t) 1
#define CMD_GEN_OVERRIDE              (uint8_t) 2
#define CMD_GEN_FOTA_VCU              (uint8_t) 3
#define CMD_GEN_FOTA_HMI              (uint8_t) 4

#define CMD_REPORT_RTC                (uint8_t) 0
#define CMD_REPORT_ODOM               (uint8_t) 1
#define CMD_REPORT_UNITID             (uint8_t) 2

#define CMD_AUDIO_BEEP                (uint8_t) 0
#define CMD_AUDIO_MUTE                (uint8_t) 1

#define CMD_FINGER_ADD                (uint8_t) 0
#define CMD_FINGER_DEL                (uint8_t) 1
#define CMD_FINGER_RST                (uint8_t) 2

#define CMD_REMOTE_PAIRING           (uint8_t) 0

// Response Status List
#define RESPONSE_STATUS_ERROR         (uint8_t) 0
#define RESPONSE_STATUS_OK            (uint8_t) 1
#define RESPONSE_STATUS_INVALID       (uint8_t) 2

// Others Parameters
#define MCU_RPM_MAX                  (uint32_t) 99999
#define VCU_ODOMETER_KM_MAX          (uint32_t) 99999
#define MCU_SPEED_KPH_MAX            (uint8_t) 150

// CAN Message Address
#define CAND_VCU_SWITCH              (uint32_t) 0x000
#define CAND_VCU_DATETIME            (uint32_t) 0x001
#define CAND_VCU_SELECT_SET          (uint32_t) 0x002
#define CAND_VCU_TRIP_MODE           (uint32_t) 0x003
#define CAND_BMS_PARAM_1             (uint32_t) 0x0B0
#define CAND_BMS_PARAM_2             (uint32_t) 0x0B1
#endif
#define CAND_BMS_SETTING             (uint32_t) 0x1B2
#define CAND_HMI1                    (uint32_t) 0x7C0
#define CAND_HMI2                    (uint32_t) 0x7D0

#if (BOOTLOADER)
// FOCAN Message Address
#define CAND_SET_PROGRESS            (uint32_t) 0x101
#define CAND_GET_CHECKSUM            (uint32_t) 0x102
#define CAND_PRA_DOWNLOAD            (uint32_t) 0x103
#define CAND_INIT_DOWNLOAD           (uint32_t) 0x104
#define CAND_DOWNLOADING             (uint32_t) 0x105
#define CAND_PASCA_DOWNLOAD          (uint32_t) 0x106
#endif

/* Exported typedef ----------------------------------------------------------*/
typedef enum {
  IAP_VCU = 0xA1B2C3D4,
  IAP_HMI = 0X1A2B3C4D
} IAP_TYPE;

typedef enum {
  IAP_SIMCOM_TIMEOUT = 0x035F67B8,
  IAP_DOWNLOAD_ERROR = 0x6AA55122,
  IAP_FIRMWARE_SAME = 0xA5BE5FF3,
  IAP_CHECKSUM_INVALID = 0xD7E9EF0D,
  IAP_CANBUS_FAILED = 0x977FC0A3,
  IAP_DFU_ERROR = 0xFA359EA1,
  IAP_DFU_SUCCESS = 0x41B0FC9E,
  IAP_RESPONSE_COUNT = 7
} IAP_RESPONSE;

#if (BOOTLOADER)
typedef enum {
  FOCAN_ERROR = 0x00,
  FOCAN_ACK = 0x79,
  FOCAN_NACK = 0x1F
} FOCAN;

#else
typedef enum {
  PAYLOAD_RESPONSE = 0,
  PAYLOAD_REPORT,
  PAYLOAD_MAX = 1,
} PAYLOAD_TYPE;

/* Exported struct --------------------------------------------------------------*/
typedef struct {
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  int8_t tzQuarterHour;
} timestamp_t;

#endif
#endif /* DEFINES_SHARED_H_ */
