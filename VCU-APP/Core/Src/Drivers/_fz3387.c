/*
 * _finger_ada.c
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

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_fz3387.h"
#include "DMA/_dma_finger.h"

/* Private variables
 * -----------------------------------------------------------*/
static packet_t PKT;

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t FINGER_IO_WRITE(uint8_t *data, uint8_t len);
static uint8_t writeStructuredPacket(void);
static uint8_t getStructuredPacket(void);
static uint8_t sendCmdPacket(uint8_t *data, uint8_t size);
static void setPacket(uint8_t type, uint16_t length, uint8_t *data);

/* Public functions implementation
 * ---------------------------------------------*/
/**************************************************************************/
/*!
 @brief  Verifies the sensors' access password (default password is 0x0000000).
 A good way to also check if the sensors is active and responding
 @returns True if password is correct
 */
/**************************************************************************/
uint8_t fz3387_checkPassword(void) {
	uint8_t data[] = {
			FINGERPRINT_VERIFYPASSWORD,
			(uint8_t)(FINGERPRINT_PASSWORD >> 24),
			(uint8_t)(FINGERPRINT_PASSWORD >> 16),
			(uint8_t)(FINGERPRINT_PASSWORD >> 8),
			(uint8_t)(FINGERPRINT_PASSWORD & 0xFF)
	};

	if (sendCmdPacket(data, sizeof(data)) != FINGERPRINT_OK)
		return FINGERPRINT_PACKETRECIEVEERR;

	if (PKT.data[0] == FINGERPRINT_OK)
		return FINGERPRINT_OK;
	else
		return FINGERPRINT_PACKETRECIEVEERR;
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to take an image of the finger pressed on surface
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_NOFINGER</code> if no finger detected
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 @returns <code>FINGERPRINT_IMAGEFAIL</code> on imaging error
 */
/**************************************************************************/
uint8_t fz3387_getImage(void) {
	uint8_t data[] = {FINGERPRINT_GETIMAGE};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to convert image to feature template
 @param slot Location to place feature template (put one in 1 and another in 2
 for verification to create model)
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_IMAGEMESS</code> if image is too messy
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 @returns <code>FINGERPRINT_FEATUREFAIL</code> on failure to identify
 fingerprint features
 @returns <code>FINGERPRINT_INVALIDIMAGE</code> on failure to identify
 fingerprint features
 */
uint8_t fz3387_image2Tz(uint8_t slot) {
	uint8_t data[] = {
			FINGERPRINT_IMAGE2TZ,
			slot
	};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to take two print feature template and create a model
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 @returns <code>FINGERPRINT_ENROLLMISMATCH</code> on mismatch of fingerprints
 */
uint8_t fz3387_createModel(void) {
	uint8_t data[] = {FINGERPRINT_REGMODEL};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to store the calculated model for later matching
 @param   location The model location #
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to
 flash memory
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_storeModel(uint16_t location) {
	uint8_t data[] = {
			FINGERPRINT_STORE,
			0x01,
			(uint8_t)(location >> 8),
			(uint8_t)(location & 0xFF)
	};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to load a fingerprint model from flash into buffer 1
 @param   location The model location #
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_loadModel(uint16_t location) {
	uint8_t data[] = {
			FINGERPRINT_LOAD,
			0x01,
			(uint8_t)(location >> 8),
			(uint8_t)(location & 0xFF)
	};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to transfer 256-byte fingerprint template from the
 buffer to the UART
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_getModel(void) {
	uint8_t data[] = {
			FINGERPRINT_UPLOAD,
			0x01
	};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to delete a model in memory
 @param   location The model location #
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to
 flash memory
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_deleteModel(uint16_t location) {
	uint8_t data[] = {
			FINGERPRINT_DELETE,
			(uint8_t)(location >> 8),
			(uint8_t)(location & 0xFF),
			0x00,
			0x01
	};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to delete ALL models in memory
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to
 flash memory
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_emptyDatabase(void) {
	uint8_t data[] = {FINGERPRINT_EMPTY};
	return sendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to search the current slot 1 fingerprint features to
 match saved templates. The matching location is stored in <b>fingerID</b> and
 the matching fingerConfidence in <b>fingerConfidence</b>
 @returns <code>FINGERPRINT_OK</code> on fingerprint match success
 @returns <code>FINGERPRINT_NOTFOUND</code> no match made
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t fz3387_fingerFastSearch(uint16_t *id, uint16_t *confidence) {
	uint8_t data[] = {
			FINGERPRINT_HISPEEDSEARCH,
			0x01,
			0x00,
			0x00,
			0x00,
			0xA3
	};

	// high speed search of slot #1 starting at page 0x0000 and page #0x00A3
	sendCmdPacket(data, sizeof(data));
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
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t fz3387_getTemplateCount(uint16_t *templateCount) {
	uint8_t data[] = {FINGERPRINT_TEMPLATECOUNT};
	sendCmdPacket(data, sizeof(data));

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
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t fz3387_setPassword(uint32_t password) {
	uint8_t data[] = {
			FINGERPRINT_SETPASSWORD,
			(password >> 24),
			(password >> 16),
			(password >> 8),
			password
	};
	return sendCmdPacket(data, sizeof(data));
}

/* Private functions implementation
 * ---------------------------------------------*/
/**************************************************************************/
/*!
 @brief   Helper function to process a packet and send it over UART to the
 sensor
 @param   packet A structure containing the bytes to transmit
 */
/**************************************************************************/
static uint8_t writeStructuredPacket(void) {
	uint8_t buf[128], i=0;

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

	return FINGER_IO_WRITE(buf, i);
}

/**************************************************************************/
/*!
 @brief   Helper function to receive data over UART from the sensor and process
 it into a packet
 @param   packet A structure containing the bytes received
 @param   timeout how many milliseconds we're willing to wait
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_TIMEOUT</code> or <code>FINGERPRINT_BADPACKET</code>
 on failure
 */
/**************************************************************************/
static uint8_t getStructuredPacket(void) {
	uint8_t byte;
	uint16_t idx = 0;

	while (1) {
		byte = FINGER_UART_RX[idx];

		switch (idx) {
		case 0:
			if (byte != (FINGERPRINT_STARTCODE >> 8)) {
				return FINGERPRINT_BADPACKET;
			}
			PKT.start_code = (uint16_t)byte << 8;
			break;
		case 1:
			PKT.start_code |= byte;
			if (PKT.start_code != FINGERPRINT_STARTCODE) {
				return FINGERPRINT_BADPACKET;
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
				return FINGERPRINT_BADPACKET;
			}

			PKT.data[idx - 9] = byte;
			if ((idx - 8) == PKT.length) {
				return FINGERPRINT_OK;
			}
			break;
		}
		idx++;
	}

	// Shouldn't get here so...
	return FINGERPRINT_BADPACKET;
}

/**************************************************************************/
/*!
 @brief Send command packet
 */
/**************************************************************************/
static uint8_t sendCmdPacket(uint8_t *data, uint8_t size) {
	memset(&PKT, 0, sizeof(PKT));
	setPacket(FINGERPRINT_COMMANDPACKET, size, data);

	FINGER_Reset_Buffer();
	if (!writeStructuredPacket()){
		return FINGERPRINT_PACKETRECIEVEERR;
	}

	memset(&PKT, 0, sizeof(PKT));
	if (getStructuredPacket() != FINGERPRINT_OK) {
		return FINGERPRINT_PACKETRECIEVEERR;
	}
	if (PKT.type != FINGERPRINT_ACKPACKET) {
		return FINGERPRINT_PACKETRECIEVEERR;
	}
	return PKT.data[0];
}

/**************************************************************************/
/*!
 @brief Packet conversion
 */
/**************************************************************************/
static void setPacket(uint8_t type, uint16_t length, uint8_t *data) {
	PKT.start_code = FINGERPRINT_STARTCODE;
	PKT.type = type;
	PKT.length = length;
	PKT.address[0] = (uint8_t)(FINGERPRINT_ADDRESS >> 24);
	PKT.address[1] = (uint8_t)(FINGERPRINT_ADDRESS >> 16);
	PKT.address[2] = (uint8_t)(FINGERPRINT_ADDRESS >> 8);
	PKT.address[3] = (uint8_t)(FINGERPRINT_ADDRESS & 0xFF);

	memcpy(PKT.data, data, (length < 64) ? length : 64);
}

static uint8_t FINGER_IO_WRITE(uint8_t *data, uint8_t len) {
	uint8_t ok;
	uint32_t tick;

	//#if FINGER_DEBUG
	//	printf("FGR: TX => ");
	//	printf_hex((char *)data, len);
	//	printf("\n");
	//#endif

	ok = FINGER_Transmit(data, len);

	if (ok) {
		tick = _GetTickMS();
		do {
			ok = FINGER_Received();
		} while(!ok && _GetTickMS() - tick < 500);
	}

	return ok;
}

