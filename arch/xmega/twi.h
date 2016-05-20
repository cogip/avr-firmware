/*
 * xmega_twi.h
 *
 *  Created on: 29 janv. 2015
 *      Author: ldo
 */

#ifndef XMEGA_TWI_H_
#define XMEGA_TWI_H_

#include <avr/interrupt.h>

void
xmega_twi_master_setup (TWI_t *twi, uint16_t freq);

void
xmega_twi_master_write_handler (TWI_t *twi);

void
xmega_twi_write (TWI_t *twi, uint8_t slave_address, uint8_t *writeData,
		 uint8_t bytesToWrite);

/**
 * \fn
 * \brief
 * \param twi
 * \param slave_address
 * \param write_data
 * \param read_data
 * \param nb_byte_to_read
 */
void
xmega_twi_read (TWI_t *twi, uint8_t slave_address, uint8_t *write_data,
		uint8_t *read_data, uint8_t nb_byte_to_read);

#endif /* XMEGA_TWI_H_ */
