/*
 * _log.h
 *
 *  Created on: Sep 23, 2019
 *      Author: Puja
 */

#ifndef LOG_H_
#define LOG_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define HEX_CHARS                                "0123456789ABCDEF"

/* Public functions prototype ------------------------------------------------*/
void LOG_Char(char ch);
void LOG_Enter(void);
void LOG_Int(int32_t num);
void LOG_Int0(int32_t num);
void LOG_Hex8(uint8_t num);
void LOG_Hex16(uint16_t num);
void LOG_Hex32(uint32_t num);
void LOG_Str(char *str);
void LOG_StrLn(char *str);
void LOG_Buf(char *buf, uint16_t bufsize);
void LOG_BufPrintable(char *buf, uint16_t bufsize, char subst);
void LOG_BufHex(char *buf, uint16_t bufsize);
void LOG_BufHexFancy(char *buf, uint16_t bufsize, uint8_t column_width, char subst);

#endif /* LOG_H_ */
