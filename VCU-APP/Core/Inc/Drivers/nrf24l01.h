/*
 * nrf24l01.h
 *
 *  Created on: Sep 19, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__NRF24L01_H_
#define INC_DRIVERS__NRF24L01_H_

/* Includes
 * --------------------------------------------*/
#include "App/util.h"

/* Exported constants
 * --------------------------------------------*/
#define NRF_DATA_LENGTH ((uint8_t)16)  // Max: 32 bytes
#define NRF_ADDR_LENGTH ((uint8_t)5)   // Range 3:5
#define NRF_PAIR_LENGTH (NRF_DATA_LENGTH + NRF_ADDR_LENGTH)

/* Exported enums
 * --------------------------------------------*/
typedef enum { NRF_OK, NRF_ERROR, NRF_INVALID_ARGUMENT } NRFR;

typedef enum {
  NRF_DATA_RATE_250KBPS = 1,
  NRF_DATA_RATE_1MBPS = 0,
  NRF_DATA_RATE_2MBPS = 2
} NRF_DATA_RATE;

typedef enum {
  NRF_TX_PWR_M18dBm = 0,
  NRF_TX_PWR_M12dBm = 1,
  NRF_TX_PWR_M6dBm = 2,
  NRF_TX_PWR_0dBm = 3
} NRF_TX_PWR;

typedef enum {
  NRF_ADDR_WIDTH_3 = 1,
  NRF_ADDR_WIDTH_4 = 2,
  NRF_ADDR_WIDTH_5 = 3
} NRF_ADDR_WIDTH;

typedef enum { NRF_CRC_WIDTH_1B = 0, NRF_CRC_WIDTH_2B = 1 } NRF_CRC_WIDTH;

typedef enum { NRF_STATE_RX = 1, NRF_STATE_TX = 0 } NRF_TXRX_STATE;

/* Public functions prototype
 * --------------------------------------------*/
void nrf_param(SPI_HandleTypeDef *hspi, uint8_t *rx_buffer);
NRFR nrf_change_mode(const uint8_t *tx_address, const uint8_t *rx_address,
                     uint8_t payload_width);
NRFR nrf_configure(void);
NRFR nrf_check(void);

/* CMD */
NRFR nrf_read_rx_payload(uint8_t *data);
NRFR nrf_write_tx_payload(const uint8_t *data);
NRFR nrf_write_tx_payload_noack(const uint8_t *data);
NRFR nrf_flush_rx(void);
NRFR nrf_flush_tx(void);
NRFR nrf_set_data_rate(NRF_DATA_RATE rate);
NRFR nrf_set_tx_power(NRF_TX_PWR pwr);
NRFR nrf_set_ccw(uint8_t activate);
NRFR nrf_read_carrier_detect(uint8_t *reg);
NRFR nrf_clear_interrupts(void);
NRFR nrf_set_rf_channel(uint8_t ch);
NRFR nrf_set_retransmittion_count(uint8_t count);
NRFR nrf_set_retransmittion_delay(uint8_t delay);
NRFR nrf_set_address_width(NRF_ADDR_WIDTH width);
NRFR nrf_set_rx_pipes(uint8_t pipes);
NRFR nrf_disable_auto_ack(void);
NRFR nrf_enable_auto_ack(uint8_t pipe);
NRFR nrf_enable_crc(uint8_t activate);
NRFR nrf_set_crc_width(NRF_CRC_WIDTH width);
NRFR nrf_power_up(uint8_t power_up);
NRFR nrf_rx_tx_control(NRF_TXRX_STATE rx);
NRFR nrf_enable_rx_data_ready_irq(uint8_t activate);
NRFR nrf_enable_tx_data_sent_irq(uint8_t activate);
NRFR nrf_enable_max_retransmit_irq(uint8_t activate);
NRFR nrf_set_rx_address_p0(const uint8_t *address);  // 5bytes of address
NRFR nrf_set_rx_address_p1(const uint8_t *address);  // 5bytes of address
NRFR nrf_set_tx_address(const uint8_t *address);     // 5bytes of address
NRFR nrf_set_rx_payload_width_p0(uint8_t width);
NRFR nrf_set_rx_payload_width_p1(uint8_t width);

/* EXTI Interrupt Handler
 *
 * You must call this function on Falling edge trigger detection interrupt
 * handler, typically, from HAL_GPIO_EXTI_Callback  */
uint8_t nrf_irq_handler(void);
void nrf_clear_pending_irq(void);

/* Blocking Data Receiving
 *
 * Blocks until the data has arrived, then returns a pointer to received data.
 * Please note, once nrf_packet_received_callback routine is overridden, this
 * one will stop working. */
// NRFR nrf_receive_packet(uint8_t *data, uint16_t ms);

/* Blocking Data Sending
 *
 * If the AA is enabled (default), this method will return:
 *   NRF_OK - the data has been acknowledged by other party
 *   NRF_ERROR - the data has not been received (maximum retransmissions has
 * occurred) If the AA is disabled, returns NRF_OK once the data has been
 * transmitted (with no guarantee the data was actually received). */
// NRFR nrf_send_packet(const uint8_t *data);

/* Blocking Data Sending, with NO_ACK flag
 *
 * Disables the AA for this packet, thus this method always returns NRF_OK */
NRFR nrf_send_packet_noack(const uint8_t *data);
uint8_t nrf_tx_busy(void);

/* Non-Blocking Data Sending */
// NRFR nrf_push_packet(const uint8_t *data);

#endif /* INC_DRIVERS__NRF24L01_H_ */
