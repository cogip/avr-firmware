#ifndef TWI_H_
#define TWI_H_

#include <stdint.h>
#if defined(__AVR__)
#include <avr/interrupt.h>

typedef TWI_t twi_t;
#else
typedef void twi_t;

char _twi_[1];
#define TWIC (_twi_[0])
#endif

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
