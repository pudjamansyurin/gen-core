/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DATABASE_H_
#define DATABASE_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Exported macro functions --------------------------------------------------*/
#define BIT(x)                                  (1ULL << x)
#define BV(var, x)                              (var |= (1ULL << x))
#define BC(var, x)                              (var &= ~(1ULL << x))
#define BT(var, x)                              (var ^= (1ULL << x))
#define _L(var, x)                              (var << x)
#define _R(var, x)                              (var >> x)
#define _R1(var, x)                             ((var >> x) & 0x01)
#define _R2(var, x)                             ((var >> x) & 0x03)
#define _R8(var, x)                             ((var >> x) & 0xFF)

/* Exported constants --------------------------------------------------------*/
#define EEPROM_RESET                            15U
#define RTC_RESET                               1U

#define VCU_FIRMWARE_VERSION                    "0.7"
#define VCU_VENDOR                              "GEN Indonesia"
#define VCU_BUILD_YEAR                          20U

//#define NET_SERVER_PORT                         5044
#define NET_SERVER_IP                           "pujakusumae-31974.portmap.io"
#define NET_SERVER_PORT                         31974
//#define NET_SERVER_IP                           "0.tcp.ngrok.io"
//#define NET_SERVER_PORT                         18698
#define NET_APN                                 "3gprs"                 // "telkomsel"
#define NET_APN_USERNAME                        "3gprs"                 // "wap"
#define NET_APN_PASSWORD                        "3gprs"                 // "wap123"
#define NET_BOOT_TIMEOUT                        7000                    // in ms
#define NET_REPEAT_DELAY                        5000                    // in ms
#define NET_EXTRA_TIME_MS                       1000                    // in ms

#define PREFIX_REPORT                           "@R"
#define PREFIX_COMMAND                          "@C"
#define PREFIX_ACK                              "@A"
#define PREFIX_NACK                             "@N"

#define FINGER_CONFIDENCE_MIN                   10
#define FINGER_SCAN_TIMEOUT                     10                      // in second

#define RPT_INTERVAL_SIMPLE                     5                       // in second
#define RPT_INTERVAL_FULL                       20                      // in second
#define RPT_INTERVAL_INDEPENDENT                60U											// in second
#define RPT_UNITID                              354313U

// Payload list (Keyless)
#define KEYLESS_MSG_BROADCAST                   BIT(0)
#define KEYLESS_MSG_FINDER                      BIT(1)
#define KEYLESS_MSG_SEAT                        BIT(2)

// Events group (for Frame Report)
#define EV_VCU_NETWORK_RESTART                 BIT(0)
#define EV_VCU_BIKE_FALLING                    BIT(1)
#define EV_VCU_BIKE_CRASHED                    BIT(2)
#define EV_VCU_KEYLESS_MISSING                 BIT(3)
#define EV_VCU_INDEPENDENT                     BIT(4)
#define EV_BMS_DISCHARGE_OVER_CURRENT          BIT(30)
#define EV_BMS_CHARGE_OVER_CURRENT             BIT(31)
#define EV_BMS_SHORT_CIRCUIT                   BIT(32)
#define EV_BMS_DISCHARGE_OVER_TEMPERATURE      BIT(33)
#define EV_BMS_DISCHARGE_UNDER_TEMPERATURE     BIT(34)
#define EV_BMS_CHARGE_OVER_TEMPERATURE         BIT(35)
#define EV_BMS_CHARGE_UNDER_TEMPERATURE        BIT(36)
#define EV_BMS_UNDER_VOLTAGE                   BIT(37)
#define EV_BMS_OVER_VOLTAGE                    BIT(38)
#define EV_BMS_OVER_DISCHARGE_CAPACITY         BIT(39)
#define EV_BMS_UNBALANCE                       BIT(40)
#define EV_BMS_SYSTEM_FAILURE                  BIT(41)

// Events (for Individual Thread)
#define EVT_MASK															0x7FFFFFFFUL
#define EVT_AUDIO_BEEP                        BIT(0)
#define EVT_AUDIO_BEEP_START                  BIT(1)
#define EVT_AUDIO_BEEP_STOP                   BIT(2)
#define EVT_AUDIO_MUTE_ON                     BIT(3)
#define EVT_AUDIO_MUTE_OFF                    BIT(4)
#define EVT_FINGER_PLACED                     BIT(0)
#define EVT_FINGER_ADD												BIT(1)
#define EVT_FINGER_DEL												BIT(2)
#define EVT_FINGER_RST												BIT(3)
#define EVT_COMMAND_ERROR											BIT(0)
#define EVT_COMMAND_OK												BIT(1)
#define EVT_CAN_RX_IT                         BIT(0)
#define EVT_KEYLESS_RX_IT                     BIT(0)
#define EVT_MANAGER_BMS_IRQ                   BIT(0)
#define EVT_MANAGER_KNOB_IRQ                  BIT(1)
#define EVT_SWITCH_TRIGGERED									BIT(0)

// Events group (for All Threads)
#define EVENT_MASK															0xFFFFFFUL
#define EVENT_READY                             BIT(0)

// Command Code List
#define CMD_CODE_GEN                            0
#define CMD_CODE_REPORT                         1
#define CMD_CODE_AUDIO                          2
#define CMD_CODE_FINGER                         3

// Command Sub-Code List
#define CMD_GEN_INFO                            0
#define CMD_GEN_LED                             1
#define CMD_GEN_KNOB                            2

#define CMD_REPORT_RTC                          0
#define CMD_REPORT_ODOM                         1
#define CMD_REPORT_UNITID                       2

#define CMD_AUDIO_BEEP                          0
#define CMD_AUDIO_MUTE                          1
#define CMD_AUDIO_VOL                           2

#define CMD_FINGER_ADD                          0
#define CMD_FINGER_DEL                          1
#define CMD_FINGER_RST                          2

// Response Status List
#define RESPONSE_STATUS_ERROR                   0
#define RESPONSE_STATUS_OK                      1
#define RESPONSE_STATUS_INVALID                 2

// Others Parameters
#define MCU_SPEED_MAX                           255U
#define MCU_RPM_MAX                             99999U
#define VCU_ODOMETER_MAX                        99999U

#define DRIVER_ID_NONE													0xFF

/* Exported enum ----------------------------------------------------------------*/
typedef enum {
	PAYLOAD_RESPONSE = 0,
	PAYLOAD_REPORT = 1,
	PAYLOAD_MAX = 1,
} PAYLOAD_TYPE;

/* Exported struct --------------------------------------------------------------*/
typedef struct {
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
} timestamp_t;

typedef struct {
	timestamp_t timestamp;
	RTC_DateTypeDef calibration;
} rtc_t;

typedef struct {
	struct {
		uint8_t knob;
		uint32_t unit_id;
		uint8_t independent;
		uint16_t interval;
		uint8_t volume;
		uint16_t bat_voltage;
		uint8_t signal_percent;
		uint8_t speed;
		uint32_t odometer;
		rtc_t rtc;
		uint64_t events;
		struct {
			uint32_t keyless;
		//      uint32_t finger;
		} tick;
		struct {
			uint16_t report;
			uint16_t response;
		} seq_id;
	} vcu;
} db_t;

/* Public functions implementation --------------------------------------------*/
void DB_Init(void);
void DB_SetEvent(uint64_t event_id, uint8_t value);
uint8_t DB_ReadEvent(uint64_t event_id);
void DB_VCU_CheckKnob(void);
void DB_VCU_CheckMainPower(void);
uint8_t DB_ValidThreadFlag(uint32_t flag);
uint8_t DB_ValidEventFlag(uint32_t flag);
//void DB_SetEvents(uint64_t value);
//uint8_t DB_ReadEvent(uint64_t event_id);

#endif /* DATABASE_H_ */
