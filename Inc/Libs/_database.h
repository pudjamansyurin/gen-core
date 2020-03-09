/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include "main.h"
#include "cmsis_os.h"

// macro to manipulate bit
#define BIT(x)                                  (1 << x)
#define BV(var, x)                              (var |= (1 << x))
#define BC(var, x)                              (var &= ~(1 << x))
#define BT(var, x)                              (var ^= (1 << x))
#define BSL(var, x)                             (var << x)
#define BSR(var, x)                             ((var >> x) & 0xFF)

// GLOBAL CONFIG
#define VCU_FIRMWARE_VERSION                    "0.7"
#define VCU_VENDOR                              "GEN Indonesia"
#define VCU_BUILD_YEAR                          20U

#define NET_SERVER_IP                           "36.81.133.99"
#define NET_SERVER_PORT                         5044
#define NET_APN                                 "3gprs"                 // "telkomsel"
#define NET_APN_USERNAME                        "3gprs"                 // "wap"
#define NET_APN_PASSWORD                        "3gprs"                 // "wap123"
#define NET_SIGNAL                              2                       // 2=AUTO, 13=2G, 14=3G
#define NET_BOOT_TIMEOUT                        10                      // in second
#define NET_REPEAT_DELAY                        5                       // in second
#define NET_EXTRA_TIME_MS                       500                     // in ms

#define NET_COMMAND_PREFIX                      "$T"
#define NET_ACK_PREFIX                          "@C"

#define FINGER_CONFIDENCE_MIN                   10
#define FINGER_SCAN_TIMEOUT                     20                      // in second
#define REPORT_INTERVAL_SIMPLE                  5                       // in second
#define REPORT_INTERVAL_FULL                    20                      // in second
#define REPORT_UNITID                           354313U
#define FRAME_PREFIX                            0x4047                  // "@G"

#define GMT_TIME                                7                       // Asia/Jakarta

// Event List (RTOS Tasks)
#define EVENT_IOT_RESPONSE                      BIT(0)
#define EVENT_IOT_REPORT                        BIT(1)

#define EVENT_REPORTER_CRASH                    BIT(0)
#define EVENT_REPORTER_FALL                     BIT(1)
#define EVENT_REPORTER_FALL_FIXED               BIT(2)

#define EVENT_AUDIO_BEEP                        BIT(0)
#define EVENT_AUDIO_MUTE_ON                     BIT(1)
#define EVENT_AUDIO_MUTE_OFF                    BIT(2)

#define EVENT_FINGER_PLACED                     BIT(0)

#define EVENT_CAN_RX_IT                         BIT(0)

#define EVENT_KEYLESS_RX_IT                     BIT(0)

// Payload list (Keyless)
#define KEYLESS_MSG_BROADCAST                   BIT(0)
#define KEYLESS_MSG_FINDER                      BIT(1)
#define KEYLESS_MSG_SEAT                        BIT(2)

// Events group (Frame Report)
#define REPORT_NETWORK_RESTART                  BIT(0)
#define REPORT_BIKE_FALLING                     BIT(1)
#define REPORT_BIKE_CRASHED                     BIT(2)
#define REPORT_KEYLESS_MISSING                  BIT(3)

// Command Code List
#define CMD_CODE_GEN                            0
#define CMD_CODE_REPORT                         1
#define CMD_CODE_AUDIO                          2
#define CMD_CODE_FINGER                         3
#define CMD_CODE_HMI2                           4

// Command Sub-Code List
#define CMD_GEN_INFO                            0
#define CMD_GEN_LED                             1

#define CMD_REPORT_RTC                          0
#define CMD_REPORT_ODOM                         1
#define CMD_REPORT_UNITID                       2

#define CMD_AUDIO_BEEP                          0
#define CMD_AUDIO_MUTE                          1
#define CMD_AUDIO_VOL                           2

#define CMD_FINGER_ADD                          0
#define CMD_FINGER_DEL                          1
#define CMD_FINGER_RST                          2

#define CMD_HMI2_SHUTDOWN                       0

// Response Status List
#define RESPONSE_STATUS_ERROR                   0
#define RESPONSE_STATUS_OK                      1
#define RESPONSE_STATUS_INVALID                 2

// EXTI list
#define SW_K_SELECT 					        0
#define SW_K_SET 						        1
#define SW_K_SEIN_LEFT 					        2
#define SW_K_SEIN_RIGHT 				        3
#define SW_K_REVERSE		 			        4
#define SW_K_ABS				 		        5

// enum list
typedef enum {
  SW_M_DRIVE = 0,
  SW_M_TRIP = 1,
  SW_M_REPORT = 2,
  SW_M_MAX = 2
} sw_mode_t;

typedef enum {
  SW_M_DRIVE_E = 0,
  SW_M_DRIVE_S = 1,
  SW_M_DRIVE_P = 2,
  SW_M_DRIVE_R = 3,
  SW_M_DRIVE_MAX = 2
} sw_mode_drive_t;

typedef enum {
  SW_M_TRIP_A = 0,
  SW_M_TRIP_B = 1,
  SW_M_TRIP_MAX = 1
} sw_mode_trip_t;

typedef enum {
  SW_M_REPORT_RANGE = 0,
  SW_M_REPORT_AVERAGE = 1,
  SW_M_REPORT_MAX = 1
} sw_mode_report_t;

typedef struct {
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
} timestamp_t;

typedef struct {
  timestamp_t timestamp;
  RTC_DateTypeDef calibration;
} rtc_t;

//FIXME active disabled GPIO input
typedef struct {
  //	uint8_t abs;
  //	uint8_t mirror;
  uint8_t lamp;
  uint8_t warning;
  uint8_t temperature;
  uint8_t finger;
  uint8_t keyless;
  uint8_t daylight;
//	uint8_t sein_left;
//	uint8_t sein_right;
} status_t;

typedef struct {
  uint8_t listening;
  struct {
    sw_mode_t val;
    struct {
      uint8_t val[SW_M_MAX + 1];
      uint8_t max[SW_M_MAX + 1];
      uint8_t report[SW_M_REPORT_MAX + 1];
      uint32_t trip[SW_M_TRIP_MAX + 1];
    } sub;
  } mode;
} sw_runner_t;

// Node struct
typedef struct {
  struct {
    uint8_t signal;
    uint8_t speed;
    uint32_t odometer;
    rtc_t rtc;
    struct {
      uint8_t count;
      struct {
        char event[20];
        uint16_t pin;
        GPIO_TypeDef *port;
        uint8_t state;
      } list[6];
      struct {
        uint32_t start;
        uint8_t running;
        uint8_t time;
      } timer[2];
      sw_runner_t runner;
    } sw;
  } vcu;
  struct {
    status_t status;
  } hmi1;
  struct {
    uint8_t shutdown;
  } hmi2;
} db_t;

#endif /* DATABASE_H_ */
