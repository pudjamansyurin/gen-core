/*
 * _focan.h
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

#ifndef INC_LIBS__FOCAN_H_
#define INC_LIBS__FOCAN_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_fota.h"

/* Exported constants --------------------------------------------------------*/
#define FOCAN_ACK                     (uint8_t) 0x79
#define FOCAN_NACK                    (uint8_t) 0x1F

// CAN Command Address
#define CAND_ENTER_IAP               (uint16_t) 0x00
#define CAND_GET_VERSION             (uint16_t) 0x01

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_Upgrade(void);
#endif /* INC_LIBS__FOCAN_H_ */
