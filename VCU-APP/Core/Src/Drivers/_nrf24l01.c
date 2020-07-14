/*
 * _nrf24l01.c
 *
 *  Created on: Sep 19, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Drivers/_nrf24l01.h"
#include "Libs/_keyless.h"

/* External variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;

/* Public variables -----------------------------------------------------------*/
nrf24l01 nrf;

/* Private functions prototype ------------------------------------------------*/
static void csn_set(nrf24l01 *dev);
static void csn_reset(nrf24l01 *dev);

/* Public functions implementation ---------------------------------------------*/
void ce_set(nrf24l01 *dev) {
    HAL_GPIO_WritePin(dev->config.ce_port, dev->config.ce_pin, GPIO_PIN_SET);
}

void ce_reset(nrf24l01 *dev) {
    HAL_GPIO_WritePin(dev->config.ce_port, dev->config.ce_pin, GPIO_PIN_RESET);
}

NRF_RESULT nrf_set_config(nrf24l01_config *config) {
    config->data_rate = NRF_DATA_RATE_250KBPS;
    config->tx_power = NRF_TX_PWR_0dBm;
    config->crc_width = NRF_CRC_WIDTH_1B;
    config->retransmit_count = 0x0F;   // maximum is 15 times
    config->retransmit_delay = 0x0F; // 4000us, LSB:250us
    config->rf_channel = 110;

    config->spi = &hspi1;
    config->spi_timeout = 100; // milliseconds
    config->csn_port = INT_KEYLESS_CSN_GPIO_Port;
    config->csn_pin = INT_KEYLESS_CSN_Pin;
    config->ce_port = INT_KEYLESS_CE_GPIO_Port;
    config->ce_pin = INT_KEYLESS_CE_Pin;
    config->irq_port = INT_KEYLESS_IRQ_GPIO_Port;
    config->irq_pin = INT_KEYLESS_IRQ_Pin;

    return NRF_OK;
}

NRF_RESULT nrf_init(nrf24l01 *dev, nrf24l01_config *config) {
    dev->config = *config;
    uint8_t config_reg = 0;

    // check hardware
    NRF_RESULT result = NRF_OK;
    do {
        LOG_StrLn("NRF:Init");

        // turn on the mosfet
        HAL_GPIO_WritePin(INT_KEYLESS_PWR_GPIO_Port, INT_KEYLESS_PWR_Pin, 0);
        _DelayMS(500);
        HAL_GPIO_WritePin(INT_KEYLESS_PWR_GPIO_Port, INT_KEYLESS_PWR_Pin, 1);
        _DelayMS(500);

        result = nrf_check(&nrf);

        if (result == NRF_ERROR) {
            _DelayMS(1000);
        }
    } while (result == NRF_ERROR);

    // enter standby I mode
    ce_reset(dev);
    nrf_power_up(dev, 1);

    // wait for powerup
    while ((config_reg & 2) == 0) {
        nrf_read_register(dev, NRF_CONFIG, &config_reg);
    }

    // address width
    nrf_set_address_width(dev, dev->config.addr_width);
    // openWritingPipe
    nrf_set_tx_address(dev, dev->config.tx_address);
    // openReadingPipe
    nrf_set_rx_payload_width_p0(dev, dev->config.payload_length);
    nrf_set_rx_address_p0(dev, dev->config.rx_address);
    // enable data pipe0
    nrf_set_rx_pipes(dev, 0x01);

    // CRC
    nrf_enable_crc(dev, 1);
    nrf_set_crc_width(dev, dev->config.crc_width);
    // channel
    nrf_set_rf_channel(dev, dev->config.rf_channel);
    // data rate
    nrf_set_data_rate(dev, dev->config.data_rate);
    // tx power
    nrf_set_tx_power(dev, dev->config.tx_power);

    // retransmission (auto-ack ON)
    nrf_set_retransmittion_count(dev, dev->config.retransmit_count);
    nrf_set_retransmittion_delay(dev, dev->config.retransmit_delay);
    // auto ack (Enhanced ShockBurst) on pipe0
    nrf_enable_auto_ack(dev, 0x00);
    //	nrf_disable_auto_ack(dev);

    // clear interrupt
    nrf_clear_interrupts(dev);
    // set interrupt
    nrf_enable_rx_data_ready_irq(dev, 1);
    nrf_enable_tx_data_sent_irq(dev, 1);
    nrf_enable_max_retransmit_irq(dev, 1);

    // set as PRX
    nrf_rx_tx_control(dev, NRF_STATE_RX);
    // clear FIFO
    nrf_flush_tx(dev);
    nrf_flush_rx(dev);
    // exit standby mode, now become RX MODE
    ce_set(dev);

    return NRF_OK;
}

// Checks the presence of the nRF24L01
NRF_RESULT nrf_check(nrf24l01 *dev) {
    char *nRF24_TEST_ADDR = "nRF24";
    uint8_t rxbuf[sizeof(nRF24_TEST_ADDR) - 1U];
    uint8_t *ptr = (uint8_t*) nRF24_TEST_ADDR;
    uint8_t idx;

    // Write the test address to the TX_ADDR register
    nrf_write_register_mb(dev, NRF_TX_ADDR, ptr, sizeof(nRF24_TEST_ADDR) - 1U);
    // Read it back to the buffer
    nrf_read_register_mb(dev, NRF_TX_ADDR, rxbuf, sizeof(nRF24_TEST_ADDR) - 1U);

    // Compare transmitted and received data...
    for (idx = 0U; idx < sizeof(nRF24_TEST_ADDR) - 1U; idx++) {
        if (rxbuf[idx] != *ptr++) {
            // The transceiver is absent
            return NRF_ERROR;
        }
    }

    // The transceiver is present
    return NRF_OK;
}

NRF_RESULT nrf_send_command(nrf24l01 *dev, NRF_COMMAND cmd, const uint8_t *tx,
        uint8_t *rx, uint8_t len) {
    uint8_t myTX[len + 1];
    uint8_t myRX[len + 1];
    myTX[0] = cmd;

    int i = 0;
    for (i = 0; i < len; i++) {
        myTX[1 + i] = tx[i];
        myRX[i] = 0;
    }

    csn_reset(dev);

    /* Wait for SPIx Busy flag */
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY))
        ;

    if (HAL_SPI_TransmitReceive(dev->config.spi, myTX, myRX, 1 + len,
            dev->config.spi_timeout) != HAL_OK) {
        return NRF_ERROR;
    }

    for (i = 0; i < len; i++) {
        rx[i] = myRX[1 + i];
    }

    csn_set(dev);

    return NRF_OK;
}

uint8_t nrf_send_command_single(nrf24l01 *dev, uint8_t data) {
    uint8_t rx;
    /* Wait for SPIx Busy flag */
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) != RESET)
        ;
    //	Tx buffer empty flag
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE) == RESET)
        ;

    HAL_SPI_TransmitReceive(dev->config.spi, &data, &rx, 1,
            dev->config.spi_timeout);

    return rx;
}

NRF_RESULT nrf_read_register(nrf24l01 *dev, uint8_t reg, uint8_t *data) {
    uint8_t tx = 0;
    if (nrf_send_command(dev, NRF_CMD_R_REGISTER | reg, &tx, data, 1)
            != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_read_register_mb(nrf24l01 *dev, uint8_t reg, uint8_t *data,
        uint8_t count) {
    csn_reset(dev);
    nrf_send_command_single(dev, NRF_CMD_R_REGISTER | reg);
    while (count--) {
        *data++ = nrf_send_command_single(dev, NRF_CMD_NOP);
    }
    csn_set(dev);
    return NRF_OK;
}

NRF_RESULT nrf_write_register(nrf24l01 *dev, uint8_t reg, uint8_t *data) {
    uint8_t rx = 0;
    if (nrf_send_command(dev, NRF_CMD_W_REGISTER | reg, data, &rx, 1)
            != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_write_register_mb(nrf24l01 *dev, uint8_t reg, uint8_t *data,
        uint8_t count) {
    csn_reset(dev);
    nrf_send_command_single(dev, NRF_CMD_W_REGISTER | reg);
    while (count--) {
        nrf_send_command_single(dev, *data++);
    }
    csn_set(dev);
    return NRF_OK;
}

NRF_RESULT nrf_read_rx_payload(nrf24l01 *dev, uint8_t *data) {
    uint8_t tx[dev->config.payload_length];
    if (nrf_send_command(dev, NRF_CMD_R_RX_PAYLOAD, tx, data,
            dev->config.payload_length) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_write_tx_payload(nrf24l01 *dev, const uint8_t *data) {
    uint8_t rx[dev->config.payload_length];
    if (nrf_send_command(dev, NRF_CMD_W_TX_PAYLOAD, data, rx,
            dev->config.payload_length) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_write_tx_payload_noack(nrf24l01 *dev, const uint8_t *data) {
    uint8_t rx[dev->config.payload_length];
    if (nrf_send_command(dev, NRF_CMD_W_TX_PAYLOAD_NOACK, data, rx,
            dev->config.payload_length) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_flush_tx(nrf24l01 *dev) {
    uint8_t rx = 0;
    uint8_t tx = 0;
    if (nrf_send_command(dev, NRF_CMD_FLUSH_TX, &tx, &rx, 0) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_flush_rx(nrf24l01 *dev) {
    uint8_t rx = 0;
    uint8_t tx = 0;
    if (nrf_send_command(dev, NRF_CMD_FLUSH_RX, &tx, &rx, 0) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_set_data_rate(nrf24l01 *dev, NRF_DATA_RATE rate) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
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
    if (nrf_write_register(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.data_rate = rate;
    return NRF_OK;
}

NRF_RESULT nrf_set_tx_power(nrf24l01 *dev, NRF_TX_PWR pwr) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    reg &= 0xF9;     // clear bits 1,2
    reg |= pwr << 1; // set bits 1,2
    if (nrf_write_register(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.tx_power = pwr;
    return NRF_OK;
}

NRF_RESULT nrf_set_ccw(nrf24l01 *dev, uint8_t activate) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    if (activate) {
        reg |= 0x80;
    } else {
        reg &= 0x7F;
    }

    if (nrf_write_register(dev, NRF_RF_SETUP, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_read_carrier_detect(nrf24l01 *dev, uint8_t *reg) {
    if (nrf_read_register(dev, NRF_CD, reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_clear_interrupts(nrf24l01 *dev) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_STATUS, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    reg |= 7 << 4; // setting bits 4,5,6

    if (nrf_write_register(dev, NRF_STATUS, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_set_rf_channel(nrf24l01 *dev, uint8_t ch) {
    ch &= 0x7F;
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_RF_CH, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    reg |= ch; // setting channel

    if (nrf_write_register(dev, NRF_RF_CH, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.rf_channel = ch;
    return NRF_OK;
}

NRF_RESULT nrf_set_retransmittion_count(nrf24l01 *dev, uint8_t count) {
    count &= 0x0F;
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    reg &= 0xF0;  // clearing bits 0,1,2,3
    reg |= count; // setting count

    if (nrf_write_register(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.retransmit_count = count;
    return NRF_OK;
}

NRF_RESULT nrf_set_retransmittion_delay(nrf24l01 *dev, uint8_t delay) {
    delay &= 0x0F;
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    reg &= 0x0F;       // clearing bits 1,2,6,7
    reg |= delay << 4; // setting delay

    if (nrf_write_register(dev, NRF_SETUP_RETR, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.retransmit_delay = delay;
    return NRF_OK;
}

NRF_RESULT nrf_set_address_width(nrf24l01 *dev, NRF_ADDR_WIDTH width) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_SETUP_AW, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    reg &= 0x03;  // clearing bits 0,1
    reg |= width; // setting delay

    if (nrf_write_register(dev, NRF_SETUP_AW, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.addr_width = width;
    return NRF_OK;
}

NRF_RESULT nrf_set_rx_pipes(nrf24l01 *dev, uint8_t pipes) {
    if (nrf_write_register(dev, NRF_EN_RXADDR, &pipes) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_disable_auto_ack(nrf24l01 *dev) {
    if (nrf_write_register(dev, NRF_EN_AA, 0x00) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_enable_auto_ack(nrf24l01 *dev, uint8_t pipe) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_EN_AA, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    reg |= 1 << pipe;

    if (nrf_write_register(dev, NRF_EN_AA, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_enable_crc(nrf24l01 *dev, uint8_t activate) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    if (activate) {
        reg |= 1 << 3;
    } else {
        reg &= ~(1 << 3);
    }

    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_set_crc_width(nrf24l01 *dev, NRF_CRC_WIDTH width) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    if (width == NRF_CRC_WIDTH_2B) {
        reg |= 1 << 2;
    } else {
        reg &= ~(1 << 3);
    }

    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.crc_width = width;
    return NRF_OK;
}

NRF_RESULT nrf_power_up(nrf24l01 *dev, uint8_t power_up) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    if (power_up) {
        reg |= 1 << 1;
    } else {
        reg &= ~(1 << 1);
    }

    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_rx_tx_control(nrf24l01 *dev, NRF_TXRX_STATE rx) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    if (rx) {
        reg |= 1;
    } else {
        reg &= ~(1);
    }

    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_enable_rx_data_ready_irq(nrf24l01 *dev, uint8_t activate) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }

    if (!activate) {
        reg |= 1 << 6;
    } else {
        reg &= ~(1 << 6);
    }

    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_enable_tx_data_sent_irq(nrf24l01 *dev, uint8_t activate) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    if (!activate) {
        reg |= 1 << 5;
    } else {
        reg &= ~(1 << 5);
    }
    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_enable_max_retransmit_irq(nrf24l01 *dev, uint8_t activate) {
    uint8_t reg = 0;
    if (nrf_read_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    if (!activate) {
        reg |= 1 << 4;
    } else {
        reg &= ~(1 << 4);
    }
    if (nrf_write_register(dev, NRF_CONFIG, &reg) != NRF_OK) {
        return NRF_ERROR;
    }
    return NRF_OK;
}

NRF_RESULT nrf_set_rx_address_p0(nrf24l01 *dev, const uint8_t *address) {
    uint8_t rx[5];
    if (nrf_send_command(dev, NRF_CMD_W_REGISTER | NRF_RX_ADDR_P0, address, rx,
            5) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.rx_address = address;
    return NRF_OK;
}

NRF_RESULT nrf_set_rx_address_p1(nrf24l01 *dev, const uint8_t *address) {
    uint8_t rx[5];
    if (nrf_send_command(dev, NRF_CMD_W_REGISTER | NRF_RX_ADDR_P1, address, rx,
            5) != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.rx_address = address;
    return NRF_OK;
}

NRF_RESULT nrf_set_tx_address(nrf24l01 *dev, const uint8_t *address) {
    uint8_t rx[5];
    if (nrf_send_command(dev, NRF_CMD_W_REGISTER | NRF_TX_ADDR, address, rx, 5)
            != NRF_OK) {
        return NRF_ERROR;
    }
    dev->config.tx_address = address;
    return NRF_OK;
}

NRF_RESULT nrf_set_rx_payload_width_p0(nrf24l01 *dev, uint8_t width) {
    width &= 0x3F;
    if (nrf_write_register(dev, NRF_RX_PW_P0, &width) != NRF_OK) {
        dev->config.payload_length = 0;
        return NRF_ERROR;
    }
    dev->config.payload_length = width;
    return NRF_OK;
}

NRF_RESULT nrf_set_rx_payload_width_p1(nrf24l01 *dev, uint8_t width) {
    width &= 0x3F;
    if (nrf_write_register(dev, NRF_RX_PW_P1, &width) != NRF_OK) {
        dev->config.payload_length = 0;
        return NRF_ERROR;
    }
    dev->config.payload_length = width;
    return NRF_OK;
}

NRF_RESULT nrf_send_packet(nrf24l01 *dev, const uint8_t *data) {
    dev->tx_busy = 1;

    ce_reset(dev);
    nrf_rx_tx_control(dev, NRF_STATE_TX);
    nrf_write_tx_payload(dev, data);
    ce_set(dev);

    while (dev->tx_busy == 1) {
    } // wait for end of transmition

    _DelayMS(100);
    ce_reset(dev);
    nrf_rx_tx_control(dev, NRF_STATE_RX);
    ce_set(dev);

    return dev->tx_result;
}

NRF_RESULT nrf_send_packet_noack(nrf24l01 *dev, const uint8_t *data) {
    dev->tx_busy = 1;

    ce_reset(dev);
    nrf_rx_tx_control(dev, NRF_STATE_TX);
    nrf_write_tx_payload_noack(dev, data);
    ce_set(dev);

    while (dev->tx_busy == 1) {
    } // wait for end of transmition

    _DelayMS(100);
    ce_reset(dev);
    nrf_rx_tx_control(dev, NRF_STATE_RX);
    ce_set(dev);

    return dev->tx_result;
}

NRF_RESULT nrf_receive_packet(nrf24l01 *dev, uint8_t *data, uint16_t ms) {
    NRF_RESULT ret = NRF_OK;

    dev->rx_busy = 1;

    ce_reset(dev);
    nrf_rx_tx_control(dev, NRF_STATE_RX);
    ce_set(dev);

    // wait for reception
    TickType_t tick = _GetTickMS();
    while (dev->rx_busy == 1) {
        // handle timeout
        if (_GetTickMS() - tick > ms) {
            ret = NRF_ERROR;
            break;
        }
    }

    return ret;
}

NRF_RESULT nrf_push_packet(nrf24l01 *dev, const uint8_t *data) {

    if (dev->tx_busy == 1) {
        nrf_flush_tx(dev);
    } else {
        dev->tx_busy = 1;
    }

    ce_reset(dev);
    nrf_rx_tx_control(dev, NRF_STATE_TX);
    nrf_write_tx_payload(dev, data);
    ce_set(dev);

    return NRF_OK;
}

void nrf_irq_handler(nrf24l01 *dev) {
    uint8_t status = 0;

    // read interrupt register
    if (nrf_read_register(dev, NRF_STATUS, &status) != NRF_OK) {
        return;
    }

    if ((status & (1 << 6))) { // RX FIFO Interrupt
        uint8_t fifo_status = 0;

        ce_reset(dev);
        nrf_write_register(dev, NRF_STATUS, &status);
        nrf_read_register(dev, NRF_FIFO_STATUS, &fifo_status);

        if ((fifo_status & 1) == 0) {
            uint8_t *rx_buffer = dev->config.rx_buffer;
            nrf_read_rx_payload(dev, rx_buffer);

            status |= 1 << 6;
            nrf_write_register(dev, NRF_STATUS, &status);
            nrf_flush_rx(dev);
            nrf_packet_received_callback(dev, rx_buffer);
        }
        ce_set(dev);

    }
    if ((status & (1 << 5))) { // TX Data Sent Interrupt
        status |= 1 << 5;      // clear the interrupt flag

        ce_reset(dev);
        //        nrf_rx_tx_control(dev, NRF_STATE_RX);
        //        dev->state = NRF_STATE_RX;
        nrf_write_register(dev, NRF_STATUS, &status);
        ce_set(dev);

        dev->tx_result = NRF_OK;
        dev->tx_busy = 0;
    }
    if ((status & (1 << 4))) { // MaxRetransmits reached
        status |= 1 << 4;      // clear the interrupt flag

        ce_reset(dev);
        nrf_flush_tx(dev);
        nrf_power_up(dev, 0); // power down
        nrf_power_up(dev, 1); // power up

        //        nrf_rx_tx_control(dev, NRF_STATE_RX);
        //        dev->state = NRF_STATE_RX;
        nrf_write_register(dev, NRF_STATUS, &status);
        ce_set(dev);

        dev->tx_result = NRF_ERROR;
        dev->tx_busy = 0;
    }
}

__weak void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
    // default implementation (__weak) is used in favor of nrf_receive_packet
    dev->rx_busy = 0;
}

/* Private functions implementation --------------------------------------------*/
static void csn_set(nrf24l01 *dev) {
    HAL_GPIO_WritePin(dev->config.csn_port, dev->config.csn_pin, GPIO_PIN_SET);
}

static void csn_reset(nrf24l01 *dev) {
    HAL_GPIO_WritePin(dev->config.csn_port, dev->config.csn_pin,
            GPIO_PIN_RESET);
}