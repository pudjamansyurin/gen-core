/*
 * _finger_ada.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */
/***************************************************
 This is a library for our optical Fingerprint sensor
 Designed specifically to work with the Adafruit Fingerprint sensor
 ----> http://www.adafruit.com/products/751
 These displays use TTL Serial to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef FZ3387_H_
#define FZ3387_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define FINGERPRINT_OK                  0x00
#define FINGERPRINT_PACKETRECIEVEERR    0x01
#define FINGERPRINT_NOFINGER            0x02
#define FINGERPRINT_IMAGEFAIL           0x03
#define FINGERPRINT_IMAGEMESS           0x06
#define FINGERPRINT_FEATUREFAIL         0x07
#define FINGERPRINT_NOMATCH             0x08
#define FINGERPRINT_NOTFOUND            0x09
#define FINGERPRINT_ENROLLMISMATCH      0x0A
#define FINGERPRINT_BADLOCATION         0x0B
#define FINGERPRINT_DBRANGEFAIL         0x0C
#define FINGERPRINT_UPLOADFEATUREFAIL   0x0D
#define FINGERPRINT_PACKETRESPONSEFAIL  0x0E
#define FINGERPRINT_UPLOADFAIL          0x0F
#define FINGERPRINT_DELETEFAIL          0x10
#define FINGERPRINT_DBCLEARFAIL         0x11
#define FINGERPRINT_PASSFAIL            0x13
#define FINGERPRINT_INVALIDIMAGE        0x15
#define FINGERPRINT_FLASHERR            0x18
#define FINGERPRINT_INVALIDREG          0x1A
#define FINGERPRINT_ADDRCODE            0x20
#define FINGERPRINT_PASSVERIFY          0x21

#define FINGERPRINT_STARTCODE           0xEF01

#define FINGERPRINT_COMMANDPACKET       0x1
#define FINGERPRINT_DATAPACKET          0x2
#define FINGERPRINT_ACKPACKET           0x7
#define FINGERPRINT_ENDDATAPACKET       0x8

#define FINGERPRINT_TIMEOUT             0xFF
#define FINGERPRINT_BADPACKET           0xFE

#define FINGERPRINT_GETIMAGE            0x01
#define FINGERPRINT_IMAGE2TZ            0x02
#define FINGERPRINT_REGMODEL            0x05
#define FINGERPRINT_STORE               0x06
#define FINGERPRINT_LOAD                0x07
#define FINGERPRINT_UPLOAD              0x08
#define FINGERPRINT_DELETE              0x0C
#define FINGERPRINT_EMPTY               0x0D
#define FINGERPRINT_SETPASSWORD         0x12
#define FINGERPRINT_VERIFYPASSWORD      0x13
#define FINGERPRINT_HISPEEDSEARCH       0x1B
#define FINGERPRINT_TEMPLATECOUNT       0x1D

#define FINGERPRINT_PASSWORD 	        0x0000000
#define FINGERPRINT_ADDRESS 	        0xFFFFFFFF

/* Exported struct --------------------------------------------------------------*/
typedef struct {
	uint16_t start_code;      ///< "Wakeup" code for packet detection
	uint8_t address[4];       ///< 32-bit Fingerprint sensor address
	uint8_t type;             ///< Type of packet
	uint16_t length;          ///< Length of packet
	uint8_t data[64];         ///< The raw buffer for packet payload
} packet_t;

typedef struct {
	uint16_t id;
	uint16_t confidence;
	uint16_t templateCount;
} finger_t;

/* Public functions prototype ------------------------------------------------*/
void FZ3387_SET_POWER(uint8_t state);
void FZ3387_SERIAL_WRITE(uint8_t c);
void FZ3387_SERIAL_WRITE_U16(uint16_t cc);
uint8_t FZ3387_SEND_CMD_PACKET(uint8_t *data, uint8_t size);
uint8_t FZ3387_verifyPassword(void);
uint8_t FZ3387_checkPassword(void);
uint8_t FZ3387_getImage(void);
uint8_t FZ3387_image2Tz(uint8_t slot);
uint8_t FZ3387_createModel(void);
uint8_t FZ3387_emptyDatabase(void);
uint8_t FZ3387_storeModel(uint16_t id);
uint8_t FZ3387_loadModel(uint16_t id);
uint8_t FZ3387_getModel(void);
uint8_t FZ3387_deleteModel(uint16_t id);
uint8_t FZ3387_fingerFastSearch(void);
uint8_t FZ3387_getTemplateCount(void);
uint8_t FZ3387_setPassword(uint32_t password);
void FZ3387_setPacket(uint8_t type, uint16_t length, uint8_t *data);
void FZ3387_writeStructuredPacket(void);
uint8_t FZ3387_getStructuredPacket(void);

#endif /* FZ3387_H_ */
