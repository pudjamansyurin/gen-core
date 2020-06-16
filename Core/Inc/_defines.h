/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DEFINES_H_
#define DEFINES_H_

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
#define EEPROM_RESET                            52U

#define VCU_FIRMWARE_VERSION       	            "0.7"
#define VCU_VENDOR                              "GEN"
#define VCU_BUILD_YEAR                          20U
#define VCU_UNITID                              354313U

#define NET_BOOT_TIMEOUT                        7000U               // in ms
#define NET_EXTRA_TIME                          500U                // in ms
#define NET_REPEAT_MAX							2U

#define NET_CON_APN                             "3gprs"             // "telkomsel"
#define NET_CON_USERNAME                        "3gprs"             // "wap"
#define NET_CON_PASSWORD                        "3gprs"             // "wap123"

#define NET_FTP_SERVER                          "ftp.almanshurin.com"
#define NET_FTP_USERNAME                        "fota@almanshurin.com"
#define NET_FTP_PASSWORD                        "@Garda313"

#define COMMAND_TIMEOUT                         20U                 // in seconds

#define KEYLESS_TIMEOUT						    5000U 				// in ms

#define PREFIX_REPORT                       	"@R"
#define PREFIX_COMMAND                          "@C"
#define PREFIX_ACK                              "@A"
#define PREFIX_NACK                             "@N"

#define FINGER_CONFIDENCE_MIN                   50U
#define FINGER_SCAN_TIMEOUT          	        10U                 // in second
#define FINGER_USER_MAX						    5U

#define DRIVER_ID_NONE							0xFF

#define RPT_INTERVAL_SIMPLE                     5U                  // in second
#define RPT_INTERVAL_FULL                       20U                 // in second
#define RPT_INTERVAL_INDEPENDENT                10U					// in second
#define RPT_INTERVAL_LOST				        60U					// in second
#define VCU_ACTIVATE_LOST_MODE                  120U                // in second

// Payload list (Keyless)
#define KEYLESS_MSG_BROADCAST                   BIT(0)
#define KEYLESS_MSG_FINDER                      BIT(1)
#define KEYLESS_MSG_SEAT                        BIT(2)

// Events group (for Frame Report)
#define EV_VCU_NETWORK_RESTART                 	BIT(0)
#define EV_VCU_BIKE_FALLING                    	BIT(1)
#define EV_VCU_BIKE_CRASHED                    	BIT(2)
#define EV_VCU_KEYLESS_MISSING                 	BIT(3)
#define EV_VCU_INDEPENDENT                     	BIT(4)
#define EV_VCU_UNAUTHORIZE_REMOVAL  		    BIT(5)
#define EV_BMS_DISCHARGE_OVER_CURRENT          	BIT(30)
#define EV_BMS_CHARGE_OVER_CURRENT             	BIT(31)
#define EV_BMS_SHORT_CIRCUIT                   	BIT(32)
#define EV_BMS_DISCHARGE_OVER_TEMPERATURE      	BIT(33)
#define EV_BMS_DISCHARGE_UNDER_TEMPERATURE     	BIT(34)
#define EV_BMS_CHARGE_OVER_TEMPERATURE         	BIT(35)
#define EV_BMS_CHARGE_UNDER_TEMPERATURE        	BIT(36)
#define EV_BMS_UNDER_VOLTAGE                   	BIT(37)
#define EV_BMS_OVER_VOLTAGE                    	BIT(38)
#define EV_BMS_OVER_DISCHARGE_CAPACITY         	BIT(39)
#define EV_BMS_UNBALANCE                       	BIT(40)
#define EV_BMS_SYSTEM_FAILURE                  	BIT(41)

// Events (for Individual Thread)
#define EVT_MASK								0x7FFFFFFFUL
#define EVT_IOT_DISCARD							BIT(0)
#define EVT_AUDIO_BEEP                        	BIT(0)
#define EVT_AUDIO_BEEP_START                  	BIT(1)
#define EVT_AUDIO_BEEP_STOP                   	BIT(2)
#define EVT_AUDIO_MUTE_ON                     	BIT(3)
#define EVT_AUDIO_MUTE_OFF                    	BIT(4)
#define EVT_FINGER_PLACED                    	BIT(0)
#define EVT_FINGER_ADD							BIT(1)
#define EVT_FINGER_DEL							BIT(2)
#define EVT_FINGER_RST							BIT(3)
#define EVT_COMMAND_ERROR						BIT(0)
#define EVT_COMMAND_OK							BIT(1)
#define EVT_CAN_RX_IT                         	BIT(0)
#define EVT_KEYLESS_RX_IT                     	BIT(0)
#define EVT_KEYLESS_PAIRING						BIT(1)
#define EVT_SWITCH_TRIGGERED					BIT(0)
#define EVT_SWITCH_REG_5V_IRQ                	BIT(1)
#define EVT_SWITCH_STARTER_IRQ                  BIT(2)
#define EVT_SWITCH_KNOB_IRQ                  	BIT(2)
#define EVT_HMI2POWER_CHANGED					BIT(0)

// Events group (for All Threads)
#define EVENT_MASK								0xFFFFFFUL
#define EVENT_READY                           	BIT(0)

// Command Code List
#define CMD_CODE_GEN                            0
#define CMD_CODE_REPORT                         1
#define CMD_CODE_AUDIO                          2
#define CMD_CODE_FINGER                         3
#define CMD_CODE_KEYLESS						4

// Command Sub-Code List
#define CMD_GEN_INFO                            0
#define CMD_GEN_LED                             1
#define CMD_GEN_KNOB                            2

#define CMD_REPORT_RTC                          0
#define CMD_REPORT_ODOM                         1
#define CMD_REPORT_UNITID                       2

#define CMD_AUDIO_BEEP                          0
#define CMD_AUDIO_MUTE                         	1
#define CMD_AUDIO_VOL                           2

#define CMD_FINGER_ADD                          0
#define CMD_FINGER_DEL                          1
#define CMD_FINGER_RST                          2

#define CMD_KEYLESS_PAIRING						0

// Response Status List
#define RESPONSE_STATUS_ERROR                   0
#define RESPONSE_STATUS_OK                      1
#define RESPONSE_STATUS_INVALID                 2

// CAN Message Address
#define CAND_VCU_SWITCH					 		0x000
#define CAND_VCU_DATETIME				 		0x001
#define CAND_VCU_SELECT_SET			 			0x002
#define CAND_VCU_TRIP_MODE			 			0x003
#define CAND_BMS_PARAM_1                        0x0B0
#define CAND_BMS_PARAM_2                        0x0B1
#define CAND_BMS_SETTING                        0x1B2
#define CAND_HMI1_LEFT                          0x7C0
#define CAND_HMI1_RIGHT                         0x7C1
#define CAND_HMI2                               0x7D0

// Others Parameters
#define MCU_SPEED_MAX                           255U
#define MCU_RPM_MAX                             99999U
#define VCU_ODOMETER_MAX                        99999U

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
    int8_t tzQuarterHour;
} timestamp_t;

typedef struct {
    timestamp_t timestamp;
    RTC_DateTypeDef calibration;
} rtc_t;

#endif /* DEFINES_H_ */
