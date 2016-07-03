/*
 * twi.h
 *
 *  Created on: 29 janv. 2015
 *      Author: ldo
 */

#ifndef TWI_H_
#define TWI_H_

#include <avr/interrupt.h>

typedef struct {
	volatile TWI_t *avr;
} twi_t;

void twi_master_setup(twi_t *twi, uint16_t freq);

void twi_master_write_handler(twi_t *twi);

void twi_write(twi_t *twi, uint8_t slave_address, uint8_t *write_data,
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
void twi_read(twi_t *twi, uint8_t slave_address, uint8_t *write_data,
	      uint8_t *read_data, uint8_t nb_byte_to_read);

#endif /* XMEGA_TWI_H_ */
