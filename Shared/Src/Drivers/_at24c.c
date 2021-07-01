/*
 * _at24c.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_at24c.h"

/* Private constants
 * --------------------------------------------*/
#define PAGE_SZ 32

/* Private variables
 * --------------------------------------------*/
static I2C_HandleTypeDef* i2c;
static uint16_t Dev;

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t load(uint16_t addr, uint8_t* data, uint16_t n);
static uint8_t save(uint16_t addr, uint8_t* data, uint16_t n);

/* Public functions implementation
 * --------------------------------------------*/
void AT24C_SetDevice(I2C_HandleTypeDef* hi2c, uint16_t device) {
  i2c = hi2c;
  Dev = device;
}

uint8_t AT24C_Probe(uint32_t timeout) {
  return HAL_I2C_IsDeviceReady(i2c, Dev, 10, timeout) == HAL_OK;
}


/**
 * Read sequence of n bytes
 */
uint8_t AT24C_Read(uint16_t addr, uint8_t *data, uint16_t n) {
	uint16_t c = n;
	uint8_t offD = 0;
	uint8_t ok = 0, i = 0;

	// read until are n bytes read
	while (c > 0) {
		// read maximal 32 bytes
		uint16_t nc = c;
		if (nc > 32)
			nc = 32;

		ok += load(addr, &data[offD], nc);

		addr+=nc;
		offD+=nc;
		c-=nc;
		i++;
	}

	return ok == i;
}


/**
 * Write sequence of n bytes
 */
uint8_t AT24C_Write(uint16_t addr, uint8_t *data, uint16_t n) {
	// status quo
	uint16_t c = n;						// bytes left to write
	uint8_t offD = 0;					// current offset in data pointer
	uint8_t offP;						// current offset in page
	uint16_t nc = 0;					// next n bytes to write
	uint8_t ok = 0, i = 0;

	// write all bytes in multiple steps
	while (c > 0) {
		// calc offset in page
		offP = addr % PAGE_SZ;
		// maximal 30 bytes to write
		nc = MIN(MIN(c, 30), PAGE_SZ - offP);

		ok += save(addr, &data[offD], nc);

		c-=nc;
		offD+=nc;
		addr+=nc;
		i++;
	}

	return ok == i;
}

uint8_t AT24C_Clear(uint16_t addr, uint16_t n) {
  uint8_t dummy[n];

  memset(dummy, 0, n);
  return AT24C_Write(addr, dummy, n);
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t load(uint16_t addr, uint8_t* data, uint16_t n) {
  return (HAL_I2C_Mem_Read(i2c, Dev, addr, I2C_MEMADD_SIZE_16BIT, data, n, 100) == HAL_OK);
}

static uint8_t save(uint16_t addr, uint8_t* data, uint16_t n) {
  if (HAL_I2C_Mem_Write(i2c, Dev, addr, I2C_MEMADD_SIZE_16BIT, data, n, 100) == HAL_OK) {
    _DelayMS(5);
    return 1;
  }
  return 0;
}


