/*
 *_nrf24l01.c
 *
 *Created on: Sep 19, 2019
 *     Author: Puja
 */
/*Includes ------------------------------------------------------------------*/
#include "Drivers/_nrf24l01.h"
#include "Libs/_remote.h"

/*Private variables
 *----------------------------------------------------------*/
static nrf24l01 NRF;

/*Private functions prototype
 *------------------------------------------------*/
static void csn_set(void);
static void csn_reset(void);
static void ce_set(void);
static void ce_reset(void);
static NRF_RESULT nrf_send_command(NRF_COMMAND cmd, const uint8_t *tx, uint8_t *rx, uint8_t len);
static NRF_RESULT nrf_read_register(uint8_t reg, uint8_t *data);
static NRF_RESULT nrf_read_registers(uint8_t reg, uint8_t *data, uint8_t len);
static NRF_RESULT nrf_write_register(uint8_t reg, uint8_t *data);
static NRF_RESULT nrf_write_registers(uint8_t reg, uint8_t *data, uint8_t len);

/*Public functions implementation
 *---------------------------------------------*/
void nrf_param(SPI_HandleTypeDef *hspi, uint8_t *rx_buffer)
{
	nrf24l01_config *c = &(NRF.config);

	c->addr_width = NRF_ADDR_LENGTH - 2;
	c->payload_length = NRF_DATA_LENGTH;
	c->rx_buffer = rx_buffer;
	c->data_rate = NRF_DATA_RATE_250KBPS;
	c->tx_power = NRF_TX_PWR_M18dBm;
	c->crc_width = NRF_CRC_WIDTH_1B;
	c->retransmit_count = 0x0F;	// maximum is 15 times
	c->retransmit_delay = 0x0F;	// 4000us, LSB:250us
	c->rf_channel = 110;
	c->spi = hspi;
	c->spi_timeout = 10;	// milliseconds
}

// Checks the presence of the nRF24L01
NRF_RESULT nrf_check(void)
{
	char *nRF24_TEST_ADDR = "nRF24";
	uint8_t buflen = strlen(nRF24_TEST_ADDR);
	uint8_t *txbuf = (uint8_t*) nRF24_TEST_ADDR;
	uint8_t rxbuf[buflen];

	ce_reset();
	nrf_write_registers(NRF_TX_ADDR, txbuf, buflen);
	nrf_read_registers(NRF_TX_ADDR, rxbuf, buflen);
	ce_set();

	// Compare transmitted and received data...
	return (memcmp(rxbuf, txbuf, buflen) == 0) ? NRF_OK : NRF_ERROR;
}

NRF_RESULT nrf_change_mode(const uint8_t *tx_address, const uint8_t *rx_address, uint8_t payload_width)
{
	ce_reset();
	nrf_set_tx_address(tx_address);
	nrf_set_rx_address_p0(rx_address);
	nrf_set_rx_payload_width_p0(payload_width);
	ce_set();

	return NRF_OK;
}

NRF_RESULT nrf_configure(void)
{
	nrf24l01_config *c = &(NRF.config);
	uint8_t config_reg = 0;

	// enter standby I mode
	ce_reset();
	nrf_power_up(1);

	// wait for powerup
	while ((config_reg & 2) == 0)
		nrf_read_register(NRF_CONFIG, &config_reg);

	// address width
	nrf_set_address_width(c->addr_width);
	// openWritingPipe
	//  nrf_set_tx_address(c->tx_address);
	// openReadingPipe
	//  nrf_set_rx_address_p0(c->rx_address);
	//  nrf_set_rx_payload_width_p0(c->payload_length);
	// enable data pipe0
	nrf_set_rx_pipes(0x01);

	// CRC
	nrf_enable_crc(1);
	nrf_set_crc_width(c->crc_width);
	// channel
	nrf_set_rf_channel(c->rf_channel);
	// data rate
	nrf_set_data_rate(c->data_rate);
	// tx power
	nrf_set_tx_power(c->tx_power);

	// retransmission (auto-ack ON)
	// nrf_set_retransmittion_count(c->retransmit_count);
	// nrf_set_retransmittion_delay(c->retransmit_delay);

	// auto ack (Enhanced ShockBurst) on pipe0
	//nrf_enable_auto_ack(0x00);
	nrf_disable_auto_ack();

	// clear interrupt
	nrf_clear_interrupts();
	// set interrupt
	nrf_enable_rx_data_ready_irq(1);
	nrf_enable_tx_data_sent_irq(0);
	nrf_enable_max_retransmit_irq(0);

	// set as PRX
	nrf_rx_tx_control(NRF_STATE_RX);
	// clear FIFO
	nrf_flush_tx();
	nrf_flush_rx();
	// exit standby mode, now become RX MODE
	ce_set();

	return NRF_OK;
}

NRF_RESULT nrf_read_rx_payload(uint8_t *data)
{
	uint8_t len = NRF.config.payload_length;
	uint8_t tx[len];

	if (nrf_send_command(NRF_CMD_R_RX_PAYLOAD, tx, data, len) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_write_tx_payload(const uint8_t *data)
{
	uint8_t len = NRF.config.payload_length;
	uint8_t rx[len];

	if (nrf_send_command(NRF_CMD_W_TX_PAYLOAD, data, rx, len) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_write_tx_payload_noack(const uint8_t *data)
{
	uint8_t len = NRF.config.payload_length;
	uint8_t rx[len];

	if (nrf_send_command(NRF_CMD_W_TX_PAYLOAD_NOACK, data, rx, len) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_flush_tx(void)
{
	uint8_t rx = 0;
	uint8_t tx = 0;

	if (nrf_send_command(NRF_CMD_FLUSH_TX, &tx, &rx, 0) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_flush_rx(void)
{
	uint8_t rx = 0;
	uint8_t tx = 0;

	if (nrf_send_command(NRF_CMD_FLUSH_RX, &tx, &rx, 0) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_set_data_rate(NRF_DATA_RATE rate)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_RF_SETUP, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (rate & 1)
	{
		// low bit set
		reg |= 1 << 5;
	}
	else
	{
		// low bit clear
		reg &= ~(1 << 5);
	}

	if (rate & 2)
	{
		// high bit set
		reg |= 1 << 3;
	}
	else
	{
		// high bit clear
		reg &= ~(1 << 3);
	}

	if (nrf_write_register(NRF_RF_SETUP, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.data_rate = rate;
	return NRF_OK;
}

NRF_RESULT nrf_set_tx_power(NRF_TX_PWR pwr)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_RF_SETUP, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg &= 0xF9;	// clear bits 1,2
	reg |= pwr << 1;	// set bits 1,2
	if (nrf_write_register(NRF_RF_SETUP, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.tx_power = pwr;
	return NRF_OK;
}

NRF_RESULT nrf_set_ccw(uint8_t activate)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_RF_SETUP, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (activate)
	{
		reg |= 0x80;
	}
	else
	{
		reg &= 0x7F;
	}

	if (nrf_write_register(NRF_RF_SETUP, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_read_carrier_detect(uint8_t *reg)
{
	if (nrf_read_register(NRF_CD, reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_clear_interrupts(void)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_STATUS, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg |= (NRF_RX_DR | NRF_TX_DS | NRF_MAX_RT);

	if (nrf_write_register(NRF_STATUS, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_set_rf_channel(uint8_t ch)
{
	uint8_t reg = 0;

	ch &= 0x7F;
	if (nrf_read_register(NRF_RF_CH, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg |= ch;	// setting channel
	if (nrf_write_register(NRF_RF_CH, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.rf_channel = ch;
	return NRF_OK;
}

NRF_RESULT nrf_set_retransmittion_count(uint8_t count)
{
	uint8_t reg = 0;

	count &= 0x0F;
	if (nrf_read_register(NRF_SETUP_RETR, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg &= 0xF0;	// clearing bits 0,1,2,3
	reg |= count;	// setting count

	if (nrf_write_register(NRF_SETUP_RETR, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.retransmit_count = count;
	return NRF_OK;
}

NRF_RESULT nrf_set_retransmittion_delay(uint8_t delay)
{
	uint8_t reg = 0;
	delay &= 0x0F;

	if (nrf_read_register(NRF_SETUP_RETR, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg &= 0x0F;	// clearing bits 1,2,6,7
	reg |= delay << 4;	// setting delay

	if (nrf_write_register(NRF_SETUP_RETR, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.retransmit_delay = delay;
	return NRF_OK;
}

NRF_RESULT nrf_set_address_width(NRF_ADDR_WIDTH width)
{
	uint8_t reg = 0;
	if (nrf_read_register(NRF_SETUP_AW, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg &= 0x03;	// clearing bits 0,1
	reg |= width;	// setting delay

	if (nrf_write_register(NRF_SETUP_AW, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.addr_width = width;
	return NRF_OK;
}

NRF_RESULT nrf_set_rx_pipes(uint8_t pipes)
{
	if (nrf_write_register(NRF_EN_RXADDR, &pipes) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_disable_auto_ack(void)
{
	if (nrf_write_register(NRF_EN_AA, 0x00) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_enable_auto_ack(uint8_t pipe)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_EN_AA, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	reg |= 1 << pipe;

	if (nrf_write_register(NRF_EN_AA, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_enable_crc(uint8_t activate)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (activate)
	{
		reg |= 1 << 3;
	}
	else
	{
		reg &= ~(1 << 3);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_set_crc_width(NRF_CRC_WIDTH width)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (width == NRF_CRC_WIDTH_2B)
	{
		reg |= 1 << 2;
	}
	else
	{
		reg &= ~(1 << 3);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.crc_width = width;
	return NRF_OK;
}

NRF_RESULT nrf_power_up(uint8_t power_up)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (power_up)
	{
		reg |= 1 << 1;
	}
	else
	{
		reg &= ~(1 << 1);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_rx_tx_control(NRF_TXRX_STATE rx)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (rx)
	{
		reg |= 1;
	}
	else
	{
		reg &= ~(1);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.state = rx;
	return NRF_OK;
}

NRF_RESULT nrf_enable_rx_data_ready_irq(uint8_t activate)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (!activate)
	{
		reg |= 1 << 6;
	}
	else
	{
		reg &= ~(1 << 6);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_enable_tx_data_sent_irq(uint8_t activate)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (!activate)
	{
		reg |= 1 << 5;
	}
	else
	{
		reg &= ~(1 << 5);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_enable_max_retransmit_irq(uint8_t activate)
{
	uint8_t reg = 0;

	if (nrf_read_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	if (!activate)
	{
		reg |= 1 << 4;
	}
	else
	{
		reg &= ~(1 << 4);
	}

	if (nrf_write_register(NRF_CONFIG, &reg) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

NRF_RESULT nrf_set_rx_address_p0(const uint8_t *address)
{
	uint8_t rx[5];

	if (nrf_send_command(NRF_CMD_W_REGISTER | NRF_RX_ADDR_P0, address, rx, 5) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.rx_address = address;
	return NRF_OK;
}

NRF_RESULT nrf_set_rx_address_p1(const uint8_t *address)
{
	uint8_t rx[5];

	if (nrf_send_command(NRF_CMD_W_REGISTER | NRF_RX_ADDR_P1, address, rx, 5) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.rx_address = address;
	return NRF_OK;
}

NRF_RESULT nrf_set_tx_address(const uint8_t *address)
{
	uint8_t rx[5];
	if (nrf_send_command(NRF_CMD_W_REGISTER | NRF_TX_ADDR, address, rx, 5) != NRF_OK)
	{
		return NRF_ERROR;
	}

	NRF.config.tx_address = address;
	return NRF_OK;
}

NRF_RESULT nrf_set_rx_payload_width_p0(uint8_t width)
{
	width &= 0x3F;
	if (nrf_write_register(NRF_RX_PW_P0, &width) != NRF_OK)
	{
		NRF.config.payload_length = 0;
		return NRF_ERROR;
	}

	NRF.config.payload_length = width;
	return NRF_OK;
}

NRF_RESULT nrf_set_rx_payload_width_p1(uint8_t width)
{
	width &= 0x3F;
	if (nrf_write_register(NRF_RX_PW_P1, &width) != NRF_OK)
	{
		NRF.config.payload_length = 0;
		return NRF_ERROR;
	}

	NRF.config.payload_length = width;
	return NRF_OK;
}

//NRF_RESULT nrf_send_packet(const uint8_t *data) {
//	NRF.tx_busy = 1;
//
//	ce_reset();
//	nrf_rx_tx_control(NRF_STATE_TX);
//	nrf_write_tx_payload(data);
//	ce_set();
//
//	// wait for end of transmition
//	while (NRF.tx_busy) {
//		_DelayMS(1);
//	};
//
//	ce_reset();
//	nrf_rx_tx_control(NRF_STATE_RX);
//	ce_set();
//
//	return NRF.tx_result;
//}

NRF_RESULT nrf_send_packet_noack(const uint8_t *data)
{
	NRF_RESULT res;
	//uint8_t status = 0;

	ce_reset();
	// read interrupt register
	//if (nrf_read_register(NRF_STATUS, &status) == NRF_OK) {
	//	if (status & NRF_RX_DR)
	//		nrf_flush_rx();
	//	if (status & NRF_MAX_RT)
	//		nrf_flush_tx();
	//	nrf_clear_interrupts();
	//}
	//nrf_power_up(0);
	//nrf_power_up(1);
	nrf_rx_tx_control(NRF_STATE_TX);
	res = nrf_write_tx_payload_noack(data);
	ce_set();

	_DelayMS(2);
	nrf_flush_tx();

	ce_reset();
	//nrf_power_up(0);
	//nrf_power_up(1);
	nrf_rx_tx_control(NRF_STATE_RX);
	ce_set();

	return res;
}

//NRF_RESULT nrf_push_packet(const uint8_t *data) {
//	NRF_RESULT res;
//
//	if (NRF.tx_busy == 1) {
//		nrf_flush_tx();
//	} else {
//		NRF.tx_busy = 1;
//	}

//
//	ce_reset();
//	nrf_rx_tx_control(NRF_STATE_TX);
//	res = nrf_write_tx_payload(data);
//	ce_set();
//
//	return res;
//}

//NRF_RESULT nrf_receive_packet(uint8_t *data, uint16_t ms) {
//	NRF.rx_busy = 1;
//
//	ce_reset();
//	nrf_rx_tx_control(NRF_STATE_RX);
//	ce_set();
//
//	// wait for reception
//	uint32_t tick = _GetTickMS();
//	while (_GetTickMS() - tick < ms && NRF.rx_busy)
//		_DelayMS(1);
//
//	return !NRF.rx_busy;
//}

void nrf_irq_handler(void)
{
	uint8_t status = 0;

	// read interrupt register
	if (nrf_read_register(NRF_STATUS, &status) != NRF_OK)
		return;

	if (status & NRF_RX_DR)
	{
		// RX FIFO Interrupt
		uint8_t fifo_status = 0;

		ce_reset();
		nrf_write_register(NRF_STATUS, &status);
		nrf_read_register(NRF_FIFO_STATUS, &fifo_status);

		if ((fifo_status & 1) == 0)
		{
			uint8_t *rx_buffer = NRF.config.rx_buffer;
			nrf_read_rx_payload(rx_buffer);

			status |= NRF_RX_DR;
			nrf_write_register(NRF_STATUS, &status);
			nrf_flush_rx();
			nrf_packet_received_callback(rx_buffer);
		}
		ce_set();
	}

	if (status & NRF_TX_DS)
	{
		// TX Data Sent Interrupt
		status |= NRF_TX_DS;	// clear the interrupt flag

		ce_reset();
		//		nrf_rx_tx_control(NRF_STATE_RX);
		nrf_write_register(NRF_STATUS, &status);
		ce_set();

		NRF.tx_result = NRF_OK;
		NRF.tx_busy = 0;
	}

	if (status & NRF_MAX_RT)
	{
		// MaxRetransmits reached
		status |= NRF_MAX_RT;	// clear the interrupt flag

		ce_reset();
		nrf_flush_tx();
		nrf_power_up(0);	// power down
		nrf_power_up(1);	// power up

		//		nrf_rx_tx_control(NRF_STATE_RX);
		nrf_write_register(NRF_STATUS, &status);
		ce_set();

		NRF.tx_result = NRF_ERROR;
		NRF.tx_busy = 0;
	}
}

__weak void nrf_packet_received_callback(uint8_t *data)
{
	// default implementation (__weak) is used in favor of nrf_receive_packet
	NRF.rx_busy = 0;
	RMT_PacketReceived(data);
}

/*Private functions implementation
 *--------------------------------------------*/
static void csn_set(void)
{
	GATE_RemoteCSN(GPIO_PIN_SET);
}

static void csn_reset(void)
{
	GATE_RemoteCSN(GPIO_PIN_RESET);
}

static void ce_set(void)
{
	GATE_RemoteCE(GPIO_PIN_SET);
}

static void ce_reset(void)
{
	GATE_RemoteCE(GPIO_PIN_RESET);
}

static NRF_RESULT nrf_send_command(NRF_COMMAND cmd, const uint8_t *tx, uint8_t *rx,
		uint8_t len)
{
	uint8_t myTX[len + 1];
	uint8_t myRX[len + 1];
	myTX[0] = cmd;

	int i = 0;
	for (i = 0; i < len; i++)
	{
		myTX[1 + i] = tx[i];
		myRX[i] = 0;
	}

	csn_reset();

	/*Wait for SPIx Busy flag */
	while (__HAL_SPI_GET_FLAG(NRF.config.spi, SPI_FLAG_BSY))
		;

	if (HAL_SPI_TransmitReceive(NRF.config.spi, myTX, myRX, 1 + len, NRF.config.spi_timeout) != HAL_OK)
	{
		return NRF_ERROR;
	}

	for (i = 0; i < len; i++)
	{
		rx[i] = myRX[1 + i];
	}

	csn_set();

	return NRF_OK;
}

static NRF_RESULT nrf_read_register(uint8_t reg, uint8_t *data)
{
	uint8_t tx = 0;

	if (nrf_send_command(NRF_CMD_R_REGISTER | reg, &tx, data, 1) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

static NRF_RESULT nrf_read_registers(uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t tx[len];

	for (uint8_t i=0; i<len; i++)
	{
		tx[i] = NRF_CMD_NOP;
	}

	if (nrf_send_command(NRF_CMD_R_REGISTER | reg, tx, data, len) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

static NRF_RESULT nrf_write_register(uint8_t reg, uint8_t *data)
{
	uint8_t rx = 0;

	if (nrf_send_command(NRF_CMD_W_REGISTER | reg, data, &rx, 1) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}

static NRF_RESULT nrf_write_registers(uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t rx[len];

	if (nrf_send_command(NRF_CMD_W_REGISTER | reg, data, rx, len) != NRF_OK)
	{
		return NRF_ERROR;
	}

	return NRF_OK;
}
