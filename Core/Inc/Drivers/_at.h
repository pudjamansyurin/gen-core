/*
 * _at.h
 *
 *  Created on: May 13, 2020
 *      Author: pudja
 */

#ifndef INC_LIBS__AT_H_
#define INC_LIBS__AT_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_simcom.h"

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    ATW = 0,
    ATR = 1,
} AT_MODE;

typedef enum {
    CMEE_DISABLE = 0,
    CMEE_NUMERIC = 1,
    CMEE_VERBOSE = 2
} AT_CMEE;

typedef enum {
    CSCLK_DISABLE = 0,
    CSCLK_EN_DTR = 1,
    CSCLK_EN_AUTO = 2
} AT_CSCLK;

typedef enum {
    CNMP_ACT_AUTO = 2,
    CNMP_ACT_GSM_ONLY = 13,
    CNMP_ACT_UMTS_ONLY = 14
} AT_CNMP_MODE;

typedef enum {
    CNMP_ACT_P_GSM = 13,
    CNMP_ACT_P_UMTS = 14
} AT_CNMP_PREFERRED;

typedef enum {
    CREG_MODE_DISABLE = 0,
    CREG_MODE_ENABLE = 1,
    CREG_MODE_EN_LOCATION = 2,
} AT_C_GREG_MODE;

typedef enum {
    CREG_STAT_NOT_REGISTERED = 0,
    CREG_STAT_REG_HOME = 1,
    CREG_STAT_SEARCHING = 2,
    CREG_STAT_REG_DENIED = 3,
    CREG_STAT_UNKNOWN = 4,
    CREG_STAT_REG_ROAMING = 5
} AT_C_GREG_STAT;

typedef enum {
    CGATT_DETACHED = 0,
    CGATT_ATTACHED = 1
} AT_CGATT;

typedef enum {
    CIPMODE_NORMAL = 0,
    CIPMODE_TRANSPARENT = 1,
} AT_CIPMODE;

typedef enum {
    CIPMUX_SINGLE_IP = 0,
    CIPMUX_MULTI_IP = 1,
} AT_CIPMUX;

typedef enum {
    CIPRXGET_DISABLE = 0,
    CIPRXGET_ENABLE = 1,
    CIPRXGET_EN_1460B = 2,
    CIPRXGET_EN_HEX_730B = 3,
    CIPRXGET_QUERY = 4,
} AT_CIPRXGET;

typedef enum {
    CAT_ACT_GSM = 0,
    CAT_ACT_GSM_COMPACT = 1,
    CAT_ACT_UTRAN = 2,
    CAT_ACT_GSM_EDGE = 3,
    CAT_ACT_UTRAN_HSDPA = 4,
    CAT_ACT_UTRAN_HSUPA = 5,
    CAT_ACT_UTRAN_HSDPA_AND_HSUPA = 6,
    CAT_ACT_E_UTRAN = 7
} AT_CSACT_ACT;

typedef enum {
    CIPSTAT_IP_INITIAL = 0,
    CIPSTAT_IP_START = 1,
    CIPSTAT_IP_CONFIG = 2,
    CIPSTAT_IP_GPRSACT = 3,
    CIPSTAT_IP_STATUS = 4,
    CIPSTAT_CONNECTING = 5,
    CIPSTAT_CONNECT_OK = 6,
    CIPSTAT_CLOSING = 7,
    CIPSTAT_CLOSED = 8,
    CIPSTAT_PDP_DEACT = 9,
    CIPSTAT_UNKNOWN = 10,
} AT_CIPSTATUS;

typedef enum {
    AT_DISABLE = 0,
    AT_ENABLE = 1
} AT_BOOL;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
    uint8_t rssi;
    uint8_t ber;
    uint8_t percent;
} at_csq_t;

typedef struct {
    AT_CSACT_ACT act;
    char rac[3];
    uint8_t creg;
    uint8_t cgreg;
} at_csact_t;

typedef struct {
    AT_CNMP_MODE mode;
    AT_CNMP_PREFERRED preferred;
} at_cnmp_t;

typedef struct {
    AT_C_GREG_MODE mode;
    AT_C_GREG_STAT stat;
} at_c_greg_t;

typedef struct {
    char mode[4];
    char ip[30];
    uint16_t port;
} at_cipstart_t;

typedef struct {
    char apn[20];
    char username[20];
    char password[20];
} at_cstt_t;

typedef struct {
    char address[20];
} at_cifsr_t;

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT AT_CommandEchoMode(uint8_t state);
SIMCOM_RESULT AT_SignalQualityReport(at_csq_t *signal);
SIMCOM_RESULT AT_ConnectionStatusSingle(AT_CIPSTATUS *state);
SIMCOM_RESULT AT_GetLocalIpAddress(at_cifsr_t *param);
SIMCOM_RESULT AT_StartConnectionSingle(at_cipstart_t *param);
SIMCOM_RESULT AT_ConfigureAPN(AT_MODE mode, at_cstt_t *param);
SIMCOM_RESULT AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET *state);
SIMCOM_RESULT AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX *state);
SIMCOM_RESULT AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE *state);
SIMCOM_RESULT AT_GprsAttachment(AT_MODE mode, AT_CGATT *state);
SIMCOM_RESULT AT_NetworkRegistrationStatus(AT_MODE mode, at_c_greg_t *param);
SIMCOM_RESULT AT_NetworkRegistration(AT_MODE mode, at_c_greg_t *param);
SIMCOM_RESULT AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t *param);
SIMCOM_RESULT AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t *param);
SIMCOM_RESULT AT_ShowRemoteIp(AT_MODE mode, AT_BOOL *state);
SIMCOM_RESULT AT_IpPackageHeader(AT_MODE mode, AT_BOOL *state);
SIMCOM_RESULT AT_GetLocalTimestamp(AT_MODE mode, AT_BOOL *state);
SIMCOM_RESULT AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK *state);
SIMCOM_RESULT AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE *state);
SIMCOM_RESULT AT_FixedLocalRate(AT_MODE mode, uint32_t *rate);
SIMCOM_RESULT AT_Clock(AT_MODE mode, timestamp_t *tm);
#endif /* INC_LIBS__AT_H_ */
