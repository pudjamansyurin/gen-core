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
#include "DMA/_dma_finger.h"
#include "Drivers/_fz3387.h"

/* Private variables -----------------------------------------------------------*/
static packet_t packet;

/* Private functions implementation --------------------------------------------*/
static void FINGER_IO_WRITE(uint8_t c);
static void FINGER_IO_WRITE_U16(uint16_t cc);

/* Public functions implementation ---------------------------------------------*/
/**************************************************************************/
/*!
 @brief   Helper function to process a packet and send it over UART to the sensor
 @param   packet A structure containing the bytes to transmit
 */
/**************************************************************************/
void fz3387_writeStructuredPacket(void) {
	FINGER_Reset_Buffer();

	FINGER_IO_WRITE_U16(packet.start_code);
	FINGER_IO_WRITE(packet.address[0]);
	FINGER_IO_WRITE(packet.address[1]);
	FINGER_IO_WRITE(packet.address[2]);
	FINGER_IO_WRITE(packet.address[3]);
	FINGER_IO_WRITE(packet.type);

	uint16_t wire_length = packet.length + 2;
	FINGER_IO_WRITE_U16(wire_length);

	uint16_t sum = ((wire_length) >> 8) + ((wire_length) & 0xFF) + packet.type;
	for (uint8_t i = 0; i < packet.length; i++) {
		FINGER_IO_WRITE(packet.data[i]);
		sum += packet.data[i];
	}

	FINGER_IO_WRITE_U16(sum);

	_DelayMS(250);
}

/**************************************************************************/
/*!
 @brief   Helper function to receive data over UART from the sensor and process it into a packet
 @param   packet A structure containing the bytes received
 @param   timeout how many milliseconds we're willing to wait
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_TIMEOUT</code> or <code>FINGERPRINT_BADPACKET</code> on failure
 */
/**************************************************************************/
uint8_t fz3387_getStructuredPacket(void) {
	uint8_t byte;
	uint16_t idx = 0;

	while (1) {
		byte = FINGER_UART_RX[idx];

		switch (idx) {
			case 0:
				if (byte != (FINGERPRINT_STARTCODE >> 8)) {
					// continue;
					return FINGERPRINT_BADPACKET;
				}
				packet.start_code = (uint16_t) byte << 8;
				break;
			case 1:
				packet.start_code |= byte;
				if (packet.start_code != FINGERPRINT_STARTCODE) {
					return FINGERPRINT_BADPACKET;
				}
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				packet.address[idx - 2] = byte;
				break;
			case 6:
				packet.type = byte;
				break;
			case 7:
				packet.length = (uint16_t) byte << 8;
				break;
			case 8:
				packet.length |= byte;
				break;
			default:
				packet.data[idx - 9] = byte;
				if ((idx - 8) == packet.length) {
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
uint8_t fz3387_SendCmdPacket(uint8_t *data, uint8_t size) {
	fz3387_setPacket(FINGERPRINT_COMMANDPACKET, size, data);
	fz3387_writeStructuredPacket();

	if (fz3387_getStructuredPacket() != FINGERPRINT_OK) {
		return FINGERPRINT_PACKETRECIEVEERR;
	}
	if (packet.type != FINGERPRINT_ACKPACKET) {
		return FINGERPRINT_PACKETRECIEVEERR;
	}
	return packet.data[0];
}

/**************************************************************************/
/*!
 @brief Packet conversion
 */
/**************************************************************************/

void fz3387_setPacket(uint8_t type, uint16_t length, uint8_t *data) {
	packet.start_code = FINGERPRINT_STARTCODE;
	packet.type = type;
	packet.length = length;
	packet.address[0] = (uint8_t) (FINGERPRINT_ADDRESS >> 24);
	packet.address[1] = (uint8_t) (FINGERPRINT_ADDRESS >> 16);
	packet.address[2] = (uint8_t) (FINGERPRINT_ADDRESS >> 8);
	packet.address[3] = (uint8_t) (FINGERPRINT_ADDRESS & 0xFF);

	if (length < 64)
		memcpy(packet.data, data, length);
	else
		memcpy(packet.data, data, 64);
}

/**************************************************************************/
/*!
 @brief  Verifies the sensors' access password (default password is 0x0000000). A good way to also check if the sensors is active and responding
 @returns True if password is correct
 */
/**************************************************************************/
uint8_t fz3387_verifyPassword(void) {
	return fz3387_checkPassword() == FINGERPRINT_OK;
}

uint8_t fz3387_checkPassword(void) {
	uint8_t data[] = {
			FINGERPRINT_VERIFYPASSWORD,
			(uint8_t) (FINGERPRINT_PASSWORD >> 24),
			(uint8_t) (FINGERPRINT_PASSWORD >> 16),
			(uint8_t) (FINGERPRINT_PASSWORD >> 8),
			(uint8_t) (FINGERPRINT_PASSWORD & 0xFF)
	};

	fz3387_SendCmdPacket(data, sizeof(data));
	if (packet.data[0] == FINGERPRINT_OK)
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
	uint8_t data[] = {
			FINGERPRINT_GETIMAGE
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to convert image to feature template
 @param slot Location to place feature template (put one in 1 and another in 2 for verification to create model)
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_IMAGEMESS</code> if image is too messy
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 @returns <code>FINGERPRINT_FEATUREFAIL</code> on failure to identify fingerprint features
 @returns <code>FINGERPRINT_INVALIDIMAGE</code> on failure to identify fingerprint features
 */
uint8_t fz3387_image2Tz(uint8_t slot) {
	uint8_t data[] = {
			FINGERPRINT_IMAGE2TZ,
			slot
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to take two print feature template and create a model
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 @returns <code>FINGERPRINT_ENROLLMISMATCH</code> on mismatch of fingerprints
 */
uint8_t fz3387_createModel(void) {
	uint8_t data[] = {
			FINGERPRINT_REGMODEL
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to store the calculated model for later matching
 @param   location The model location #
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to flash memory
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_storeModel(uint16_t location) {
	uint8_t data[] = {
			FINGERPRINT_STORE,
			0x01,
			(uint8_t) (location >> 8),
			(uint8_t) (location & 0xFF)
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
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
			(uint8_t) (location >> 8),
			(uint8_t) (location & 0xFF)
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to transfer 256-byte fingerprint template from the buffer to the UART
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_getModel(void) {
	uint8_t data[] = {
			FINGERPRINT_UPLOAD,
			0x01
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to delete a model in memory
 @param   location The model location #
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to flash memory
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_deleteModel(uint16_t location) {
	uint8_t data[] = {
			FINGERPRINT_DELETE,
			(uint8_t) (location >> 8),
			(uint8_t) (location & 0xFF),
			0x00,
			0x01
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to delete ALL models in memory
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
 @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to flash memory
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
uint8_t fz3387_emptyDatabase(void) {
	uint8_t data[] = {
			FINGERPRINT_EMPTY
	};
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/**************************************************************************/
/*!
 @brief   Ask the sensor to search the current slot 1 fingerprint features to match saved templates. The matching location is stored in <b>fingerID</b> and the matching fingerConfidence in <b>fingerConfidence</b>
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
	fz3387_SendCmdPacket(data, sizeof(data));
	*id = 0xFFFF;
	*confidence = 0xFFFF;

	*id = packet.data[1];
	*id <<= 8;
	*id |= packet.data[2];

	*confidence = packet.data[3];
	*confidence <<= 8;
	*confidence |= packet.data[4];

	return packet.data[0];
}

/**************************************************************************/
/*!
 @brief   Ask the sensor for the number of templates stored in memory. The number is stored in <b>fingerTemplateCount</b> on success.
 @returns <code>FINGERPRINT_OK</code> on success
 @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
 */
/**************************************************************************/
uint8_t fz3387_getTemplateCount(uint16_t *templateCount) {
	uint8_t data[] = {
			FINGERPRINT_TEMPLATECOUNT
	};
	fz3387_SendCmdPacket(data, sizeof(data));

	*templateCount = packet.data[1];
	*templateCount <<= 8;
	*templateCount |= packet.data[2];

	return packet.data[0];
}

/**************************************************************************/
/*!
 @brief   Set the password on the sensor (future communication will require password verification so don't forget it!!!)
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
	return fz3387_SendCmdPacket(data, sizeof(data));
}

/* Private functions implementation ---------------------------------------------*/
static void FINGER_IO_WRITE(uint8_t c) {
	FINGER_Transmit8(&c);
}

static void FINGER_IO_WRITE_U16(uint16_t cc) {
	FINGER_IO_WRITE((uint8_t) (cc >> 8));
	FINGER_IO_WRITE((uint8_t) (cc & 0xFF));
}
