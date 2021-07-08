/*
 * _codec.c
 *
 *  Created on: Mar 8, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_codec.h"

#include "i2c.h"

/* Private constants
 * --------------------------------------------*/
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
 // on accurate values, they just guarantee that the application will not remain
 // stuck if the SPI communication is corrupted.
 // You may modify these timeout values depending on CPU frequency and
 // application conditions (interrupts routines ...). */
#define I2Cx_TIMEOUT_MAX 0x1000
#define VERIFY_WRITTENDATA 0

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint8_t address;
  I2C_HandleTypeDef *pi2c;
} codec_t;

/* Private variables
 * --------------------------------------------*/
static codec_t CODEC = {
    .pi2c = &hi2c1,
};

/* Private functions prototype
 * --------------------------------------------*/
static void I2Cx_Error(void);
static uint8_t I2Cx_Probe(void);

/* Public functions implementations
 * --------------------------------------------*/
/**
 * @brief  Initializes Audio low level.
 */
uint8_t CODEC_Init(uint8_t Addr) {
  uint8_t ok;

  CODEC.address = Addr;
  MX_I2C1_Init();
  GATE_AudioCodecReset();
  ok = I2Cx_Probe();

  return ok;
}

/**
 * @brief  DeInitializes Audio low level.
 */
void CODEC_DeInit(void) {
  GATE_AudioCodecStop();
  HAL_I2C_DeInit(CODEC.pi2c);
}

/**
 * @brief  Writes a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @param  Value: Data to be written
 */
uint8_t CODEC_Write(uint8_t Reg, uint8_t Value) {
  uint8_t ok, result = 0;

  ok = HAL_I2C_Mem_Write(CODEC.pi2c, CODEC.address, (uint16_t)Reg,
                         I2C_MEMADD_SIZE_8BIT, &Value, 1,
                         I2Cx_TIMEOUT_MAX) == HAL_OK;

  /* Check the communication status */
  if (!ok) /* Execute user timeout callback */
    I2Cx_Error();
  else {
#ifdef VERIFY_WRITTENDATA
    /* Verify that the data has been correctly written */
    result = (CODEC_Read(Reg) == Value) ? 0 : 1;
#endif /* VERIFY_WRITTENDATA */
  }
  return result;
}

/**
 * @brief  Reads a single data.
 * @param  Addr: I2C address
 * @param  Reg: Reg address
 * @retval Data to be read
 */
uint8_t CODEC_Read(uint8_t Reg) {
  uint8_t ok, value = 0;

  ok = HAL_I2C_Mem_Read(CODEC.pi2c, CODEC.address, (uint16_t)Reg,
                        I2C_MEMADD_SIZE_8BIT, &value, 1,
                        I2Cx_TIMEOUT_MAX) == HAL_OK;

  /* Check the communication status */
  if (!ok) /* Execute user timeout callback */
    I2Cx_Error();

  return value;
}

/* Private functions implementations
 * --------------------------------------------*/

/**
 * @brief  Manages error callback by re-initializing I2C.
 * @param  Addr: I2C Address
 */
static void I2Cx_Error(void) {
  /* De-initialize the I2C communication bus */
  HAL_I2C_MspDeInit(CODEC.pi2c);

  /* Re-Initialize the I2C communication bus */
  HAL_I2C_MspInit(CODEC.pi2c);
}

static uint8_t I2Cx_Probe(void) {
  return (HAL_I2C_IsDeviceReady(CODEC.pi2c, CODEC.address, 10, 1000) == HAL_OK);
}
