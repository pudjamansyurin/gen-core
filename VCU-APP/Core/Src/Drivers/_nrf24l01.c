/*
 * _nrf24l01.c
 *
 *  Created on: Sep 19, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "Drivers/_nrf24l01.h"
#include "Libs/_remote.h"

/* Private variables ----------------------------------------------------------*/
static nrf24l01 NRF;

/* Private functions prototype ------------------------------------------------*/
static void csn_set(void);
static void csn_reset(void);
static void ce_set(void);
static void ce_reset(void);

/* Public functions implementation ---------------------------------------------*/
void nrf_param(SPI_HandleTypeDef *hspi, uint8_t *rx_buffer) {
  NRF.config.addr_width = NRF_ADDR_LENGTH - 2;
  NRF.config.payload_length = NRF_DATA_LENGTH;
  NRF.config.rx_buffer = rx_buffer;

  NRF.config.data_rate = NRF_DATA_RATE_250KBPS;
  NRF.config.tx_power = NRF_TX_PWR_M18dBm;
  NRF.config.crc_width = NRF_CRC_WIDTH_1B;
  NRF.config.retransmit_count = 0x0F;   // maximum is 15 times
  NRF.config.retransmit_delay = 0x0F; // 4000us, LSB:250us
  NRF.config.rf_channel = 110;
  NRF.config.spi = hspi;
  NRF.config.spi_timeout = 3; // milliseconds
}

NRF_RESULT nrf_init(void) {
  NRF_RESULT result;

  // check hardware
  do {
    LOG_StrLn("NRF:Init");

    // turn on the mosfet
    GATE_RemoteReset();

    // reset peripheral
    HAL_SPI_DeInit(NRF.config.spi);
    HAL_SPI_Init(NRF.config.spi);

    result = nrf_check();
  } while (result == NRF_ERROR);

  return NRF_OK;
}

void nrf_deinit(void) {
  GATE_RemoteShutdown();
  HAL_SPI_DeInit(NRF.config.spi);
}

// Checks the presence of the nRF24L01
NRF_RESULT nrf_check(void) {
  char *nRF24_TEST_ADDR = "nRF24";
  uint8_t rxbuf[sizeof(nRF24_TEST_ADDR) - 1U];
  uint8_t *ptr = (uint8_t*) nRF24_TEST_ADDR;

  // Write the test address to the TX_ADDR register
  nrf_write_register_mb(NRF_TX_ADDR, ptr, sizeof(nRF24_TEST_ADDR) - 1U);
  // Read it back to the buffer
  nrf_read_register_mb(NRF_TX_ADDR, rxbuf, sizeof(nRF24_TEST_ADDR) - 1U);

  // Compare transmitted and received data...
  for (uint8_t idx = 0U; idx < sizeof(nRF24_TEST_ADDR) - 1U; idx++)
    if (rxbuf[idx] != *ptr++)
      // The transceiver is absent
      return NRF_ERROR;

  // The transceiver is present
  return NRF_OK;
}

NRF_RESULT nrf_change_mode(const uint8_t *tx_address, const uint8_t *rx_address, uint8_t payload_width) {
  ce_reset();
  nrf_set_tx_address(tx_address);
  nrf_set_rx_address_p0(rx_address);
  nrf_set_rx_payload_width_p0(payload_width);
  ce_set();

  return NRF_OK;
}

NRF_RESULT nrf_configure(void) {
  uint8_t config_reg = 0;

  // enter standby I mode
  ce_reset();
  nrf_power_up(1);

  // wait for powerup
  while ((config_reg & 2) == 0)
    nrf_read_register(NRF_CONFIG, &config_reg);

  // address width
  nrf_set_address_width(NRF.config.addr_width);
  // openWritingPipe
  //  nrf_set_tx_address(NRF.config.tx_address);
  //  // openReadingPipe
  //  nrf_set_rx_address_p0(NRF.config.rx_address);
  //  nrf_set_rx_payload_width_p0(NRF.config.payload_length);
  // enable data pipe0
  nrf_set_rx_pipes(0x01);

  // CRC
  nrf_enable_crc(1);
  nrf_set_crc_width(NRF.config.crc_width);
  // channel
  nrf_set_rf_channel(NRF.config.rf_channel);
  // data rate
  nrf_set_data_rate(NRF.config.data_rate);
  // tx power
  nrf_set_tx_power(NRF.config.tx_power);

  // retransmission (auto-ack ON)
  nrf_set_retransmittion_count(NRF.config.retransmit_count);
  nrf_set_retransmittion_delay(NRF.config.retransmit_delay);
  // auto ack (Enhanced ShockBurst) on pipe0
  nrf_enable_auto_ack(0x00);
  //	nrf_disable_auto_ack();

  // clear interrupt
  nrf_clear_interrupts();
  // set interrupt
  nrf_enable_rx_data_ready_irq(1);
  nrf_enable_tx_data_sent_irq(1);
  nrf_enable_max_retransmit_irq(1);

  // set as PRX
  nrf_rx_tx_control(NRF_STATE_RX);
  // clear FIFO
  nrf_flush_tx();
  nrf_flush_rx();
  // exit standby mode, now become RX MODE
  ce_set();

  return NRF_OK;
}

NRF_RESULT nrf_send_command(NRF_COMMAND cmd, const uint8_t *tx,
    uint8_t *rx, uint8_t len) {
  uint8_t myTX[len + 1];
  uint8_t myRX[len + 1];
  myTX[0] = cmd;

  int i = 0;
  for (i = 0; i < len; i++) {
    myTX[1 + i] = tx[i];
    myRX[i] = 0;
  }

  csn_reset();

  /* Wait for SPIx Busy flag */
  while (__HAL_SPI_GET_FLAG(NRF.config.spi, SPI_FLAG_BSY))
    ;

  if (HAL_SPI_TransmitReceive(NRF.config.spi, myTX, myRX, 1 + len,
      NRF.config.spi_timeout) != HAL_OK) {
    return NRF_ERROR;
  }

  for (i = 0; i < len; i++) {
    rx[i] = myRX[1 + i];
  }

  csn_set();

  return NRF_OK;
}

uint8_t nrf_send_command_single(uint8_t data) {
  uint8_t rx;
  /* Wait for SPIx Busy flag */
  while (__HAL_SPI_GET_FLAG(NRF.config.spi, SPI_FLAG_BSY) != RESET)
    ;
  //	Tx buffer empty flag
  while (__HAL_SPI_GET_FLAG(NRF.config.spi, SPI_FLAG_TXE) == RESET)
    ;

  HAL_SPI_TransmitReceive(NRF.config.spi, &data, &rx, 1,
      NRF.config.spi_timeout);

  return rx;
}

NRF_RESULT nrf_read_register(uint8_t reg, uint8_t *data) {
  uint8_t tx = 0;
  if (nrf_send_command(NRF_CMD_R_REGISTER | reg, &tx, data, 1)
      != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_read_register_mb(uint8_t reg, uint8_t *data,
    uint8_t count) {
  csn_reset();
  nrf_send_command_single(NRF_CMD_R_REGISTER | reg);
  while (count--) {
    *data++ = nrf_send_command_single(NRF_CMD_NOP);
  }
  csn_set();
  return NRF_OK;
}

NRF_RESULT nrf_write_register(uint8_t reg, uint8_t *data) {
  uint8_t rx = 0;
  if (nrf_send_command(NRF_CMD_W_REGISTER | reg, data, &rx, 1)
      != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_write_register_mb(uint8_t reg, uint8_t *data,
    uint8_t count) {
  csn_reset();
  nrf_send_command_single(NRF_CMD_W_REGISTER | reg);
  while (count--) {
    nrf_send_command_single(*data++);
  }
  csn_set();
  return NRF_OK;
}

NRF_RESULT nrf_read_rx_payload(uint8_t *data) {
  uint8_t tx[NRF.config.payload_length];
  if (nrf_send_command(NRF_CMD_R_RX_PAYLOAD, tx, data,
      NRF.config.payload_length) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_write_tx_payload(const uint8_t *data) {
  uint8_t rx[NRF.config.payload_length];
  if (nrf_send_command(NRF_CMD_W_TX_PAYLOAD, data, rx,
      NRF.config.payload_length) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_write_tx_payload_noack(const uint8_t *data) {
  uint8_t rx[NRF.config.payload_length];
  if (nrf_send_command(NRF_CMD_W_TX_PAYLOAD_NOACK, data, rx,
      NRF.config.payload_length) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_flush_tx(void) {
  uint8_t rx = 0;
  uint8_t tx = 0;
  if (nrf_send_command(NRF_CMD_FLUSH_TX, &tx, &rx, 0) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_flush_rx(void) {
  uint8_t rx = 0;
  uint8_t tx = 0;
  if (nrf_send_command(NRF_CMD_FLUSH_RX, &tx, &rx, 0) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_set_data_rate(NRF_DATA_RATE rate) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_RF_SETUP, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  if (rate & 1) { // low bit set
    reg |= 1 << 5;
  } else { // low bit clear
    reg &= ~(1 << 5);
  }

  if (rate & 2) { // high bit set
    reg |= 1 << 3;
  } else { // high bit clear
    reg &= ~(1 << 3);
  }
  if (nrf_write_register(NRF_RF_SETUP, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.data_rate = rate;
  return NRF_OK;
}

NRF_RESULT nrf_set_tx_power(NRF_TX_PWR pwr) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_RF_SETUP, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  reg &= 0xF9;     // clear bits 1,2
  reg |= pwr << 1; // set bits 1,2
  if (nrf_write_register(NRF_RF_SETUP, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.tx_power = pwr;
  return NRF_OK;
}

NRF_RESULT nrf_set_ccw(uint8_t activate) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_RF_SETUP, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  if (activate) {
    reg |= 0x80;
  } else {
    reg &= 0x7F;
  }

  if (nrf_write_register(NRF_RF_SETUP, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_read_carrier_detect(uint8_t *reg) {
  if (nrf_read_register(NRF_CD, reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_clear_interrupts(void) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_STATUS, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  reg |= 7 << 4; // setting bits 4,5,6

  if (nrf_write_register(NRF_STATUS, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_set_rf_channel(uint8_t ch) {
  ch &= 0x7F;
  uint8_t reg = 0;
  if (nrf_read_register(NRF_RF_CH, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  reg |= ch; // setting channel

  if (nrf_write_register(NRF_RF_CH, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.rf_channel = ch;
  return NRF_OK;
}

NRF_RESULT nrf_set_retransmittion_count(uint8_t count) {
  count &= 0x0F;
  uint8_t reg = 0;
  if (nrf_read_register(NRF_SETUP_RETR, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  reg &= 0xF0;  // clearing bits 0,1,2,3
  reg |= count; // setting count

  if (nrf_write_register(NRF_SETUP_RETR, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.retransmit_count = count;
  return NRF_OK;
}

NRF_RESULT nrf_set_retransmittion_delay(uint8_t delay) {
  delay &= 0x0F;
  uint8_t reg = 0;
  if (nrf_read_register(NRF_SETUP_RETR, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  reg &= 0x0F;       // clearing bits 1,2,6,7
  reg |= delay << 4; // setting delay

  if (nrf_write_register(NRF_SETUP_RETR, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.retransmit_delay = delay;
  return NRF_OK;
}

NRF_RESULT nrf_set_address_width(NRF_ADDR_WIDTH width) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_SETUP_AW, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  reg &= 0x03;  // clearing bits 0,1
  reg |= width; // setting delay

  if (nrf_write_register(NRF_SETUP_AW, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.addr_width = width;
  return NRF_OK;
}

NRF_RESULT nrf_set_rx_pipes(uint8_t pipes) {
  if (nrf_write_register(NRF_EN_RXADDR, &pipes) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_disable_auto_ack(void) {
  if (nrf_write_register(NRF_EN_AA, 0x00) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_enable_auto_ack(uint8_t pipe) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_EN_AA, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  reg |= 1 << pipe;

  if (nrf_write_register(NRF_EN_AA, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_enable_crc(uint8_t activate) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  if (activate) {
    reg |= 1 << 3;
  } else {
    reg &= ~(1 << 3);
  }

  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_set_crc_width(NRF_CRC_WIDTH width) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  if (width == NRF_CRC_WIDTH_2B) {
    reg |= 1 << 2;
  } else {
    reg &= ~(1 << 3);
  }

  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.crc_width = width;
  return NRF_OK;
}

NRF_RESULT nrf_power_up(uint8_t power_up) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  if (power_up) {
    reg |= 1 << 1;
  } else {
    reg &= ~(1 << 1);
  }

  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_rx_tx_control(NRF_TXRX_STATE rx) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  if (rx) {
    reg |= 1;
  } else {
    reg &= ~(1);
  }

  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_enable_rx_data_ready_irq(uint8_t activate) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }

  if (!activate) {
    reg |= 1 << 6;
  } else {
    reg &= ~(1 << 6);
  }

  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_enable_tx_data_sent_irq(uint8_t activate) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  if (!activate) {
    reg |= 1 << 5;
  } else {
    reg &= ~(1 << 5);
  }
  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_enable_max_retransmit_irq(uint8_t activate) {
  uint8_t reg = 0;
  if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  if (!activate) {
    reg |= 1 << 4;
  } else {
    reg &= ~(1 << 4);
  }
  if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK) {
    return NRF_ERROR;
  }
  return NRF_OK;
}

NRF_RESULT nrf_set_rx_address_p0(const uint8_t *address) {
  uint8_t rx[5];
  if (nrf_send_command(NRF_CMD_W_REGISTER | NRF_RX_ADDR_P0, address, rx,
      5) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.rx_address = address;
  return NRF_OK;
}

NRF_RESULT nrf_set_rx_address_p1(const uint8_t *address) {
  uint8_t rx[5];
  if (nrf_send_command(NRF_CMD_W_REGISTER | NRF_RX_ADDR_P1, address, rx,
      5) != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.rx_address = address;
  return NRF_OK;
}

NRF_RESULT nrf_set_tx_address(const uint8_t *address) {
  uint8_t rx[5];
  if (nrf_send_command(NRF_CMD_W_REGISTER | NRF_TX_ADDR, address, rx, 5)
      != NRF_OK) {
    return NRF_ERROR;
  }
  NRF.config.tx_address = address;
  return NRF_OK;
}

NRF_RESULT nrf_set_rx_payload_width_p0(uint8_t width) {
  width &= 0x3F;
  if (nrf_write_register(NRF_RX_PW_P0, &width) != NRF_OK) {
    NRF.config.payload_length = 0;
    return NRF_ERROR;
  }
  NRF.config.payload_length = width;
  return NRF_OK;
}

NRF_RESULT nrf_set_rx_payload_width_p1(uint8_t width) {
  width &= 0x3F;
  if (nrf_write_register(NRF_RX_PW_P1, &width) != NRF_OK) {
    NRF.config.payload_length = 0;
    return NRF_ERROR;
  }
  NRF.config.payload_length = width;
  return NRF_OK;
}

NRF_RESULT nrf_send_packet(const uint8_t *data) {
  NRF.tx_busy = 1;

  ce_reset();
  nrf_rx_tx_control(NRF_STATE_TX);
  nrf_write_tx_payload(data);
  ce_set();

  // wait for end of transmition
  while (NRF.tx_busy) {
    _DelayMS(1);
  };

  ce_reset();
  nrf_rx_tx_control(NRF_STATE_RX);
  ce_set();

  return NRF.tx_result;
}

NRF_RESULT nrf_send_packet_noack(const uint8_t *data) {
  NRF.tx_busy = 1;

  ce_reset();
  nrf_rx_tx_control(NRF_STATE_TX);
  nrf_write_tx_payload_noack(data);
  ce_set();

  _DelayMS(2);
  // wait for end of transmition
  //  uint32_t tick = _GetTickMS();
  //  while (_GetTickMS() - tick <= 2) {
  //    if (!NRF.tx_busy)
  //      break;
  //    _DelayMS(1);
  //  }

  if (NRF.tx_busy)
    nrf_flush_tx();

  ce_reset();
  nrf_rx_tx_control(NRF_STATE_RX);
  ce_set();

  return NRF.tx_busy;
}

NRF_RESULT nrf_push_packet(const uint8_t *data) {

  if (NRF.tx_busy == 1) {
    nrf_flush_tx();
  } else {
    NRF.tx_busy = 1;
  }

  ce_reset();
  nrf_rx_tx_control(NRF_STATE_TX);
  nrf_write_tx_payload(data);
  ce_set();

  return NRF_OK;
}

NRF_RESULT nrf_receive_packet(uint8_t *data, uint16_t ms) {
  NRF.rx_busy = 1;

  ce_reset();
  nrf_rx_tx_control(NRF_STATE_RX);
  ce_set();

  // wait for reception
  while (NRF.rx_busy) {
  };
  //  TickType_t tick = _GetTickMS();
  //  while (_GetTickMS() - tick < ms) {
  //    if (!NRF.rx_busy)
  //      break;
  //    _DelayMS(1);
  //  }

  return !NRF.rx_busy;
}

void nrf_irq_handler(void) {
  uint8_t status = 0;

  // read interrupt register
  if (nrf_read_register(NRF_STATUS, &status) != NRF_OK)
    return;

  if ((status & (1 << 6))) { // RX FIFO Interrupt
    uint8_t fifo_status = 0;

    ce_reset();
    nrf_write_register(NRF_STATUS, &status);
    nrf_read_register(NRF_FIFO_STATUS, &fifo_status);

    if ((fifo_status & 1) == 0) {
      uint8_t *rx_buffer = NRF.config.rx_buffer;
      nrf_read_rx_payload(rx_buffer);

      status |= 1 << 6;
      nrf_write_register(NRF_STATUS, &status);
      nrf_flush_rx();
      nrf_packet_received_callback(rx_buffer);
    }
    ce_set();

  }
  if ((status & (1 << 5))) { // TX Data Sent Interrupt
    status |= 1 << 5;      // clear the interrupt flag

    ce_reset();
//    nrf_rx_tx_control(NRF_STATE_RX);
//    NRF.state = NRF_STATE_RX;
    nrf_write_register(NRF_STATUS, &status);
    ce_set();

    NRF.tx_result = NRF_OK;
    NRF.tx_busy = 0;
  }
  if ((status & (1 << 4))) { // MaxRetransmits reached
    status |= 1 << 4;      // clear the interrupt flag

    ce_reset();
    nrf_flush_tx();
    nrf_power_up(0); // power down
    nrf_power_up(1); // power up

    nrf_rx_tx_control(NRF_STATE_RX);
    NRF.state = NRF_STATE_RX;
    nrf_write_register(NRF_STATUS, &status);
    ce_set();

    NRF.tx_result = NRF_ERROR;
    NRF.tx_busy = 0;
  }
}

__weak void nrf_packet_received_callback(uint8_t *data) {
  // default implementation (__weak) is used in favor of nrf_receive_packet
  NRF.rx_busy = 0;

  RF_PacketReceived(data);
}

/* Private functions implementation --------------------------------------------*/
static void csn_set(void) {
  GATE_RemoteCSN(GPIO_PIN_SET);
}

static void csn_reset(void) {
  GATE_RemoteCSN(GPIO_PIN_RESET);
}

static void ce_set(void) {
  GATE_RemoteCE(GPIO_PIN_SET);
}

static void ce_reset(void) {
  GATE_RemoteCE(GPIO_PIN_RESET);
}
