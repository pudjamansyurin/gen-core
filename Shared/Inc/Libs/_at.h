/*
 * _at.h
 *
 *  Created on: May 13, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__AT_H_
#define INC_LIBS__AT_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_sim_state.h"
#if (APP)
#include "Drivers/_rtc.h"
#endif

/* Exported enums
 * --------------------------------------------*/
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
  CGATT_DETACHED = 0,
  CGATT_ATTACHED,
  CGATT_ForceEnumSize = MAX_ENUM_SIZE
} AT_CGATT;

#if AT_USE_SMS
typedef enum {
  CMGF_PDU = 0,
  CMGF_TEXT,
  CMGF_ForceEnumSize = MAX_ENUM_SIZE
} AT_CMGF;

typedef enum {
  CUSD_DISABLE = 0,
  CUSD_ENABLE,
  CUSD_CANCEL,
  CUSD_ForceEnumSize = MAX_ENUM_SIZE
} AT_CUSD_N;

typedef enum {
  CMGL_STAT_UNREAD = 0,
  CMGL_STAT_READ,
  CMGL_STAT_UNSENT,
  CMGL_STAT_SENT,
  CMGL_STAT_ALL,
  CMGL_ForceEnumSize = MAX_ENUM_SIZE
} AT_CMGL_STAT;

typedef enum {
  CMG_MODE_NORMAL = 0,
  CMG_MODE_CHANGE_STATUS,
  CMG_MODE_ForceEnumSize = MAX_ENUM_SIZE
} AT_CMG_MODE;

typedef enum {
  CMGD_ID = 0,
  CMGD_READ = 1,
  CMGD_READ_SENT,
  CMGD_READ_SENT_UNSENT,
  CMGD_ALL,
  CMGD_ForceEnumSize = MAX_ENUM_SIZE
} AT_CMGD;
#endif

#if AT_USE_TCP
typedef enum {
  CIPMODE_NORMAL = 0,
  CIPMODE_TRANSPARENT,
  CIPMODE_ForceEnumSize = MAX_ENUM_SIZE
} AT_CIPMODE;

typedef enum {
  CIPQSEND_NORMAL = 0,
  CIPQSEND_QUICK,
  CIPQSEND_ForceEnumSize = MAX_ENUM_SIZE
} AT_CIPQSEND;

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
#endif

#if AT_USE_FTP
typedef enum {
  SAPBR_BEARER_CLOSE = 0,
  SAPBR_BEARER_OPEN,
  SAPBR_BEARER_QUERY,
  SAPBR_PARAMETERS_SET,
  SAPBR_PARAMETERS_GET,
  SAPBR_ForceEnumSize = MAX_ENUM_SIZE
} AT_SAPBR_CMD;

typedef enum {
  SAPBR_CONNECTING = 0,
  SAPBR_CONNECTED,
  SAPBR_CLOSING,
  SAPBR_CLOSED,
  SAPBR_STATUS_ForceEnumSize = MAX_ENUM_SIZE
} AT_SAPBR_STATUS;

typedef enum {
  FTP_FINISH = 0,
  FTP_READY,
  FTP_ERROR_NET = 61,
  FTP_ERROR_DNS,
  FTP_ERROR_CONNECT,
  FTP_ERROR_TIMEOUT,
  FTP_ERROR_SERVER,
  FTP_ERROR_OPERATION_NOT_ALLOW,
  FTP_ERROR_REPLAY,
  FTP_ERROR_USER,
  FTP_ERROR_PASSWORD,
  FTP_ERROR_TYPE,
  FTP_ERROR_REST,
  FTP_ERROR_PASSIVE,
  FTP_ERROR_ACTIVE,
  FTP_ERROR_OPERATE,
  FTP_ERROR_UPLOAD,
  FTP_ERROR_DOWNLOAD,
  FTP_ERROR_QUIT,
  FTP_ForceEnumSize = MAX_ENUM_SIZE
} AT_FTP_RESPONSE;

typedef enum {
  FTPGET_OPEN = 1,
  FTPGET_READ,
  FTPGET_ForceEnumSize = MAX_ENUM_SIZE
} AT_FTPGET_MODE;

typedef enum {
  FTP_STATE_IDLE = 0,
  FTP_STATE_ESTABLISHED,
  FTP_STATE_ForceEnumSize = MAX_ENUM_SIZE
} AT_FTP_STATE;
#endif

typedef enum {
  AT_DISABLE = 0,
  AT_ENABLE,
  AT_ForceEnumSize = MAX_ENUM_SIZE
} AT_BOOL;

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint8_t rssi;
  uint8_t ber;
  uint8_t percent;
} at_csq_t;

typedef struct {
  uint16_t txlen;
  uint16_t acklen;
  uint16_t nacklen;
} at_cipack_t;

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
  char apn[20];
  char username[20];
  char password[20];
} at_cstt_t;

#if AT_USE_SMS
typedef struct {
  AT_CUSD_N n;
  char str[20];
  uint8_t dcs;
} at_cusd_t;

typedef struct {
  char storage[3];
  uint8_t used;
  uint8_t total;
} at_cpms_mem_t;

typedef struct {
  at_cpms_mem_t mem[3];
} at_cpms_t;

typedef struct {
  char mem[3];
  uint8_t index;
} at_cmti_t;

typedef struct {
  AT_CMGL_STAT stat;
  AT_CMG_MODE mode;
} at_cmgl_t;

typedef struct {
  uint8_t index;
  AT_CMG_MODE mode;
} at_cmgr_t;

typedef struct {
  uint8_t index;
  AT_CMGD delflag;
} at_cmgd_t;
#endif

#if AT_USE_TCP
typedef struct {
  char mode[4];
  char ip[30];
  uint16_t port;
} at_cipstart_t;

typedef struct {
  char address[20];
} at_cifsr_t;
#endif

#if AT_USE_FTP
typedef struct {
  AT_SAPBR_CMD cmd_type;
  AT_SAPBR_STATUS status;
  at_cstt_t con;
} at_sapbr_t;

typedef struct {
  AT_FTPGET_MODE mode;
  AT_FTP_RESPONSE response;
  uint16_t reqlength;
  uint16_t cnflength;
  char* ptr;
} at_ftpget_t;

typedef struct {
  char path[20];
  char file[20];
  uint32_t size;
  AT_FTP_RESPONSE response;
} at_ftp_t;
#endif

/* Public functions prototype
 * --------------------------------------------*/
SIMR AT_CommandEchoMode(uint8_t state);
SIMR AT_QueryTransmittedData(at_cipack_t* info);
SIMR AT_SignalQualityReport(at_csq_t* signal);
SIMR AT_ConnectionStatus(AT_CIPSTATUS* state);
SIMR AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t* param);
SIMR AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t* param);
SIMR AT_NetworkRegistration(char command[20], AT_MODE mode, at_c_greg_t* param);
SIMR AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK* state);
SIMR AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE* state);
SIMR AT_FixedLocalRate(AT_MODE mode, uint32_t* rate);
SIMR AT_GprsAttachment(AT_MODE mode, AT_CGATT* state);

#if AT_USE_CLK
SIMR AT_EnableLocalTimestamp(AT_MODE mode, AT_BOOL* state);
SIMR AT_Clock(AT_MODE mode, timestamp_t* tm);
#endif

#if AT_USE_SMS
SIMR AT_CharacterSetTE(AT_MODE mode, char* chset, uint8_t len);
SIMR AT_ServiceDataUSSD(AT_MODE mode, at_cusd_t* param, char* buf,
                        uint8_t buflen);
SIMR AT_MessageIndicationSMS(uint8_t mode, uint8_t mt);
SIMR AT_MessageFormatSMS(AT_MODE mode, AT_CMGF* state);
uint8_t AT_WaitMessageSMS(at_cmti_t* param, uint32_t timeout);
SIMR AT_StorageMessageSMS(AT_MODE mode, at_cpms_t* param);
SIMR AT_DeleteMessageSMS(at_cmgd_t* param);
SIMR AT_ReadMessageSMS(at_cmgr_t* param, char* buf, uint8_t buflen);
SIMR AT_ListMessageSMS(at_cmgl_t* param);
#endif

#if AT_USE_TCP
SIMR AT_ConfigureAPN(AT_MODE mode, at_cstt_t* param);
SIMR AT_GetLocalIpAddress(at_cifsr_t* param);
SIMR AT_StartConnection(at_cipstart_t* param);
SIMR AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET* state);
SIMR AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX* state);
SIMR AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE* state);
SIMR AT_ShowRemoteIp(AT_MODE mode, AT_BOOL* state);
SIMR AT_IpPackageHeader(AT_MODE mode, AT_BOOL* state);
SIMR AT_DataTransmitMode(AT_MODE mode, AT_CIPQSEND* state);
#endif

#if AT_USE_FTP
SIMR AT_BearerInitialize(void);
SIMR AT_BearerSettings(AT_MODE mode, at_sapbr_t* param);
SIMR AT_FtpInitialize(at_ftp_t* param);
SIMR AT_FtpFileSize(at_ftp_t* param);
SIMR AT_FtpDownload(at_ftpget_t* param);
SIMR AT_FtpCurrentState(AT_FTP_STATE* state);
SIMR AT_FtpResume(uint32_t start);
#endif
#endif /* INC_LIBS__AT_H_ */
