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
    ATR,
} AT_MODE;

typedef enum {
    CMEE_DISABLE = 0,
    CMEE_NUMERIC,
    CMEE_VERBOSE,
    CMEE_ForceEnumSize = MAX_ENUM_SIZE
} AT_CMEE;

typedef enum {
    CSCLK_DISABLE = 0,
    CSCLK_EN_DTR,
    CSCLK_EN_AUTO,
    CSCLK_ForceEnumSize = MAX_ENUM_SIZE
} AT_CSCLK;

typedef enum {
    CNMP_ACT_AUTO = 2,
    CNMP_ACT_GSM_ONLY = 13,
    CNMP_ACT_UMTS_ONLY = 14,
    CNMP_ACT_ForceEnumSize = MAX_ENUM_SIZE
} AT_CNMP_MODE;

typedef enum {
    CNMP_ACT_P_GSM = 13,
    CNMP_ACT_P_UMTS,
    CNMP_ACT_P_ForceEnumSize = MAX_ENUM_SIZE
} AT_CNMP_PREFERRED;

typedef enum {
    CREG_MODE_DISABLE = 0,
    CREG_MODE_ENABLE,
    CREG_MODE_EN_LOCATION,
    CREG_MODE_ForceEnumSize = MAX_ENUM_SIZE
} AT_C_GREG_MODE;

typedef enum {
    CREG_STAT_NOT_REGISTERED = 0,
    CREG_STAT_REG_HOME,
    CREG_STAT_SEARCHING,
    CREG_STAT_REG_DENIED,
    CREG_STAT_UNKNOWN,
    CREG_STAT_REG_ROAMING,
    CREG_STAT_ForceEnumSize = MAX_ENUM_SIZE
} AT_C_GREG_STAT;

typedef enum {
    CGATT_DETACHED = 0,
    CGATT_ATTACHED,
    CGATT_ForceEnumSize = MAX_ENUM_SIZE
} AT_CGATT;

typedef enum {
    CIPMODE_NORMAL = 0,
    CIPMODE_TRANSPARENT,
    CIPMODE_ForceEnumSize = MAX_ENUM_SIZE
} AT_CIPMODE;

typedef enum {
    CIPMUX_SINGLE_IP = 0,
    CIPMUX_MULTI_IP,
    CIPMUX_ForceEnumSize = MAX_ENUM_SIZE
} AT_CIPMUX;

typedef enum {
    CIPRXGET_DISABLE = 0,
    CIPRXGET_ENABLE,
    CIPRXGET_EN_1460B,
    CIPRXGET_EN_HEX_730B,
    CIPRXGET_QUERY,
    CIPRXGET_ForceEnumSize = MAX_ENUM_SIZE
} AT_CIPRXGET;

typedef enum {
    CAT_ACT_GSM = 0,
    CAT_ACT_GSM_COMPACT,
    CAT_ACT_UTRAN,
    CAT_ACT_GSM_EDGE,
    CAT_ACT_UTRAN_HSDPA,
    CAT_ACT_UTRAN_HSUPA,
    CAT_ACT_UTRAN_HSDPA_AND_HSUPA,
    CAT_ACT_E_UTRAN,
    CAT_ACT_ForceEnumSize = MAX_ENUM_SIZE
} AT_CSACT_ACT;

typedef enum {
    AT_DISABLE = 0,
    AT_ENABLE,
    AT_ForceEnumSize = MAX_ENUM_SIZE
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
SIMCOM_RESULT AT_GetLocalIpAddress(at_cifsr_t *param);
SIMCOM_RESULT AT_StartConnectionSingle(at_cipstart_t *param);
SIMCOM_RESULT AT_SignalQualityReport(at_csq_t *signal);
SIMCOM_RESULT AT_ConnectionStatusSingle(AT_CIPSTATUS *state);
SIMCOM_RESULT AT_ConfigureAPN(AT_MODE mode, at_cstt_t *param);
SIMCOM_RESULT AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t *param);
SIMCOM_RESULT AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t *param);
SIMCOM_RESULT AT_Clock(AT_MODE mode, timestamp_t *tm);

SIMCOM_RESULT AT_NetworkRegistration(char command[20], AT_MODE mode, at_c_greg_t *param);

SIMCOM_RESULT AT_GprsAttachment(AT_MODE mode, AT_CGATT *state);
SIMCOM_RESULT AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET *state);
SIMCOM_RESULT AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX *state);
SIMCOM_RESULT AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE *state);
SIMCOM_RESULT AT_ShowRemoteIp(AT_MODE mode, AT_BOOL *state);
SIMCOM_RESULT AT_IpPackageHeader(AT_MODE mode, AT_BOOL *state);
SIMCOM_RESULT AT_EnableLocalTimestamp(AT_MODE mode, AT_BOOL *state);
SIMCOM_RESULT AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK *state);
SIMCOM_RESULT AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE *state);
SIMCOM_RESULT AT_FixedLocalRate(AT_MODE mode, uint32_t *rate);
#endif /* INC_LIBS__AT_H_ */
