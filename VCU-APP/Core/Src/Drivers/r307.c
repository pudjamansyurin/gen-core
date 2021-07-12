/*
 * r307.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Pudja Mansyurin
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

/* Includes
 * --------------------------------------------*/
#include "Drivers/r307.h"

#include "DMA/dma_finger.h"

/* Exported constants
 * --------------------------------------------*/
#define FP_RECEIVING_MS ((uint16_t)1000)

#define FP_OK 0x00
#define FP_PACKETRECIEVEERR 0x01
#define FP_NOFINGER 0x02
#define FP_IMAGEFAIL 0x03
#define FP_IMAGEMESS 0x06
#define FP_FEATUREFAIL 0x07
#define FP_NOMATCH 0x08
#define FP_NOTFOUND 0x09
#define FP_ENROLLMISMATCH 0x0A
#define FP_BADLOCATION 0x0B
#define FP_DBRANGEFAIL 0x0C
#define FP_UPLOADFEATUREFAIL 0x0D
#define FP_PACKETRESPONSEFAIL 0x0E
#define FP_UPLOADFAIL 0x0F
#define FP_DELETEFAIL 0x10
#define FP_DBCLEARFAIL 0x11
#define FP_PASSFAIL 0x13
#define FP_INVALIDIMAGE 0x15
#define FP_FLASHERR 0x18
#define FP_INVALIDREG 0x1A
#define FP_ADDRCODE 0x20
#define FP_PASSVERIFY 0x21

#define FP_STARTCODE 0xEF01

#define FP_COMMANDPACKET 0x1
#define FP_DATAPACKET 0x2
#define FP_ACKPACKET 0x7
#define FP_ENDDATAPACKET 0x8

#define FP_TIMEOUT 0xFF
#define FP_BADPACKET 0xFE

#define FP_GETIMAGE 0x01
#define FP_IMAGE2TZ 0x02
#define FP_REGMODEL 0x05
#define FP_STORE 0x06
#define FP_LOAD 0x07
#define FP_UPLOAD 0x08
#define FP_DELETE 0x0C
#define FP_EMPTY 0x0D
#define FP_SETPASSWORD 0x12
#define FP_VERIFYPASSWORD 0x13
#define FP_HISPEEDSEARCH 0x1B
#define FP_TEMPLATECOUNT 0x1D

#define FP_PASSWORD 0x0000000
#define FP_ADDRESS 0xFFFFFFFF

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint16_t start_code;  ///< "Wakeup" code for packet detection
  uint8_t address[4];   ///< 32-bit Fingerprint sensor address
  uint8_t type;         ///< Type of packet
  uint16_t length;      ///< Length of packet
  uint8_t data[64];     ///< The raw buffer for packet payload
} packet_t;

/* Private variables
 * --------------------------------------------*/
static packet_t PKT;

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t WriteStructuredPacket(void);
static uint8_t GetStructuredPacket(void);
static uint8_t Transmit(uint8_t *data, uint8_t len);
static uint8_t SendCmdPacket(uint8_t *data, uint8_t size);
static void SetPacket(uint8_t type, uint16_t length, uint8_t *data);

/* Public functions implementation
 * --------------------------------------------*/
/**************************************************************************/
/*!
 @brief  Verifies the sensors' access password (default password is 0x0000000).
 A good way to also check if the sensors is active and responding
 @returns True if password is correct
 */
/**************************************************************************/
uint8_t R307_checkPassword(void) {
  uint8_t data[] = {FP_VERIFYPASSWORD, (uint8_t)(FP_PASSWORD >> 24),
                    (uint8_t)(FP_PASSWORD >> 16), (uint8_t)(FP_PASSWORD >> 8),
                    (uint8_t)(FP_PASSWORD & 0xFF)};

  if (SendCmdPacket(data, sizeof(data)) != FP_OK) return FP_PACKETRECIEVEERR;

  if (PKT.data[0] == FP_OK)
    return FP_OK;
  else
    return FP_PACKETRECIEVEERR;
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to take an image of the finger pressed on surface
 @returns <code>FP_OK</code> on success
 @returns <code>FP_NOFINGER</code> if no finger detected
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 @returns <code>FP_IMAGEFAIL</code> on imaging error
 */
/**************************************************************************/
uint8_t R307_getImage(void) {
  uint8_t data[] = {FP_GETIMAGE};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to convert image to feature template
 @param slot Location to place feature template (put one in 1 and another in 2
 for verification to create model)
 @returns <code>FP_OK</code> on success
 @returns <code>FP_IMAGEMESS</code> if image is too messy
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 @returns <code>FP_FEATUREFAIL</code> on failure to identify
 fingerprint features
 @returns <code>FP_INVALIDIMAGE</code> on failure to identify
 fingerprint features
 */
uint8_t R307_image2Tz(uint8_t slot) {
  uint8_t data[] = {FP_IMAGE2TZ, slot};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to take two print feature template and create a model
 @returns <code>FP_OK</code> on success
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 @returns <code>FP_ENROLLMISMATCH</code> on mismatch of fingerprints
 */
uint8_t R307_createModel(void) {
  uint8_t data[] = {FP_REGMODEL};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to store the calculated model for later matching
 @param   location The model location #
 @returns <code>FP_OK</code> on success
 @returns <code>FP_BADLOCATION</code> if the location is invalid
 @returns <code>FP_FLASHERR</code> if the model couldn't be written to
 flash memory
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
uint8_t R307_storeModel(uint16_t location) {
  uint8_t data[] = {FP_STORE, 0x01, (uint8_t)(location >> 8),
                    (uint8_t)(location & 0xFF)};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to load a fingerprint model from flash into buffer 1
 @param   location The model location #
 @returns <code>FP_OK</code> on success
 @returns <code>FP_BADLOCATION</code> if the location is invalid
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
uint8_t R307_loadModel(uint16_t location) {
  uint8_t data[] = {FP_LOAD, 0x01, (uint8_t)(location >> 8),
                    (uint8_t)(location & 0xFF)};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to transfer 256-byte fingerprint template from the
 buffer to the UART
 @returns <code>FP_OK</code> on success
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
uint8_t R307_getModel(void) {
  uint8_t data[] = {FP_UPLOAD, 0x01};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to delete a model in memory
 @param   location The model location #
 @returns <code>FP_OK</code> on success
 @returns <code>FP_BADLOCATION</code> if the location is invalid
 @returns <code>FP_FLASHERR</code> if the model couldn't be written to
 flash memory
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
uint8_t R307_deleteModel(uint16_t location) {
  uint8_t data[] = {FP_DELETE, (uint8_t)(location >> 8),
                    (uint8_t)(location & 0xFF), 0x00, 0x01};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to delete ALL models in memory
 @returns <code>FP_OK</code> on success
 @returns <code>FP_BADLOCATION</code> if the location is invalid
 @returns <code>FP_FLASHERR</code> if the model couldn't be written to
 flash memory
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
uint8_t R307_emptyDatabase(void) {
  uint8_t data[] = {FP_EMPTY};
  return SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to search the current slot 1 fingerprint features to
 match saved templates. The matching location is stored in <b>fingerID</b> and
 the matching fingerConfidence in <b>fingerConfidence</b>
 @returns <code>FP_OK</code> on fingerprint match success
 @returns <code>FP_NOTFOUND</code> no match made
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t R307_fingerFastSearch(uint16_t *id, uint16_t *confidence) {
  uint8_t data[] = {FP_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3};

  // high speed search of slot #1 starting at page 0x0000 and page #0x00A3
  SendCmdPacket(data, sizeof(data));
  *id = 0xFFFF;
  *confidence = 0xFFFF;

  *id = PKT.data[1];
  *id <<= 8;
  *id |= PKT.data[2];

  *confidence = PKT.data[3];
  *confidence <<= 8;
  *confidence |= PKT.data[4];

  return PKT.data[0];
}

/**************************************************************************/
/*!
 @brief   Ask the sensor for the number of templates stored in memory. The
 number is stored in <b>fingerTemplateCount</b> on success.
 @returns <code>FP_OK</code> on success
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t R307_getTemplateCount(uint16_t *templateCount) {
  uint8_t data[] = {FP_TEMPLATECOUNT};
  SendCmdPacket(data, sizeof(data));

  *templateCount = PKT.data[1];
  *templateCount <<= 8;
  *templateCount |= PKT.data[2];

  return PKT.data[0];
}

/**************************************************************************/
/*!
 @brief   Set the password on the sensor (future communication will require
 password verification so don't forget it!!!)
 @param   password 32-bit password code
 @returns <code>FP_OK</code> on success
 @returns <code>FP_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t R307_setPassword(uint32_t password) {
  uint8_t data[] = {FP_SETPASSWORD, (password >> 24), (password >> 16),
                    (password >> 8), password};
  return SendCmdPacket(data, sizeof(data));
}

/* Private functions implementation
 * --------------------------------------------*/
/**************************************************************************/
/*!
 @brief   Helper function to process a packet and send it over UART to the
 sensor
 @param   packet A structure containing the bytes to transmit
 */
/**************************************************************************/
static uint8_t WriteStructuredPacket(void) {
  uint8_t buf[128], i = 0;

  buf[i++] = (PKT.start_code >> 8);
  buf[i++] = (PKT.start_code & 0xFF);
  buf[i++] = (PKT.address[0]);
  buf[i++] = (PKT.address[1]);
  buf[i++] = (PKT.address[2]);
  buf[i++] = (PKT.address[3]);
  buf[i++] = (PKT.type);

  uint16_t wire_length = PKT.length + 2;
  buf[i++] = (wire_length >> 8);
  buf[i++] = (wire_length & 0xFF);

  uint16_t sum = ((wire_length) >> 8) + ((wire_length)&0xFF) + PKT.type;
  for (uint8_t j = 0; j < PKT.length; j++) {
    buf[i++] = PKT.data[j];
    sum += PKT.data[j];
  }
  buf[i++] = (sum >> 8);
  buf[i++] = (sum & 0xFF);

  return Transmit(buf, i);
}

/**************************************************************************/
/*!
 @brief   Helper function to receive data over UART from the sensor and process
 it into a packet
 @param   packet A structure containing the bytes received
 @param   timeout how many milliseconds we're willing to wait
 @returns <code>FP_OK</code> on success
 @returns <code>FP_TIMEOUT</code> or <code>FP_BADPACKET</code>
 on failure
 */
/**************************************************************************/
static uint8_t GetStructuredPacket(void) {
  uint8_t byte;
  uint16_t idx = 0;

  while (1) {
    byte = FINGER_UART_RX[idx];

    switch (idx) {
      case 0:
        if (byte != (FP_STARTCODE >> 8)) {
          return FP_BADPACKET;
        }
        PKT.start_code = (uint16_t)byte << 8;
        break;
      case 1:
        PKT.start_code |= byte;
        if (PKT.start_code != FP_STARTCODE) {
          return FP_BADPACKET;
        }
        break;
      case 2:
      case 3:
      case 4:
      case 5:
        PKT.address[idx - 2] = byte;
        break;
      case 6:
        PKT.type = byte;
        break;
      case 7:
        PKT.length = (uint16_t)byte << 8;
        break;
      case 8:
        PKT.length |= byte;
        break;
      default:
        if ((idx - 8) > sizeof(PKT.data)) {
          return FP_BADPACKET;
        }

        PKT.data[idx - 9] = byte;
        if ((idx - 8) == PKT.length) {
          return FP_OK;
        }
        break;
    }
    idx++;
  }

  // Shouldn't get here so...
  return FP_BADPACKET;
}

/**************************************************************************/
/*!
 @brief Send command packet
 */
/**************************************************************************/
static uint8_t SendCmdPacket(uint8_t *data, uint8_t size) {
  memset(&PKT, 0, sizeof(PKT));
  SetPacket(FP_COMMANDPACKET, size, data);

  FINGER_Reset_Buffer();
  if (!WriteStructuredPacket()) {
    return FP_PACKETRECIEVEERR;
  }

  memset(&PKT, 0, sizeof(PKT));
  if (GetStructuredPacket() != FP_OK) {
    return FP_PACKETRECIEVEERR;
  }
  if (PKT.type != FP_ACKPACKET) {
    return FP_PACKETRECIEVEERR;
  }
  return PKT.data[0];
}

/**************************************************************************/
/*!
 @brief Packet conversion
 */
/**************************************************************************/
static void SetPacket(uint8_t type, uint16_t length, uint8_t *data) {
  PKT.start_code = FP_STARTCODE;
  PKT.type = type;
  PKT.length = length;
  PKT.address[0] = (uint8_t)(FP_ADDRESS >> 24);
  PKT.address[1] = (uint8_t)(FP_ADDRESS >> 16);
  PKT.address[2] = (uint8_t)(FP_ADDRESS >> 8);
  PKT.address[3] = (uint8_t)(FP_ADDRESS & 0xFF);

  memcpy(PKT.data, data, (length < 64) ? length : 64);
}

static uint8_t Transmit(uint8_t *data, uint8_t len) {
  uint8_t ok;
  uint32_t tick;

#if FINGER_DEBUG
  printf("FGR: TX => ");
  printf_hex((char *)data, len);
  printf("\n");
#endif

  ok = FINGER_Transmit(data, len);

  if (ok) {
    tick = tickMs();
    do {
      ok = FINGER_Received();
    } while (!ok && tickIn(tick, FP_RECEIVING_MS));
  }

  return ok;
}
