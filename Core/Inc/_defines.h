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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
#define RTOS_ENABLE                             0
#define VCU_BOOTLOADER_VERSION       	        0x01

#define NET_BOOT_TIMEOUT                        7000U               // in ms
#define NET_EXTRA_TIME                          1000U               // in ms
#define NET_REPEAT_MAX							2U

#define NET_CON_APN                             "3gprs"             // "telkomsel"
#define NET_CON_USERNAME                        "3gprs"             // "wap"
#define NET_CON_PASSWORD                        "3gprs"             // "wap123"

#define NET_FTP_SERVER                          "ftp.genmotorcycles.com"
#define NET_FTP_USERNAME                        "fota@genmotorcycles.com"
#define NET_FTP_PASSWORD                        "@Garda313"

#endif /* DEFINES_H_ */
