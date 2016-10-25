//#include <avr/io.h>
//#include <util/delay.h>

#include "twi.h"

//static twi_t *twi_ref_on_twic;

/*! TWIC Master Interrupt vector */
//ISR(TWIC_TWIM_vect)
//{
//	/* If master write interrupt. */
//	if (TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm)
//		twi_master_write_handler(twi_ref_on_twic);
//
//	/* If master read interrupt. */
//	else if (TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm)
//		; /* TWI_MasterReadHandler(TWIC); */
//}

//static volatile uint8_t twi_i;
//static uint8_t twi_nb_bytes_to_write;
//static uint8_t *twi_write_data;

/**
 * RXACK : When read as zero, the most recent acknowledge bit from the slave
 * was ACK, and when read as one the most recent acknowledge bit was NACK
 */
void twi_master_write_handler(twi_t *twi)
{
#if 0
	if (twi->MASTER.STATUS & TWI_MASTER_RXACK_bm)
		/* received slave non-acknowledge (NACK) : send STOP */
		twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

	else if (twi_i < twi_nb_bytes_to_write) {
		/* more bytes to write : send data */
		uint8_t data = twi_write_data[twi_i];

		twi->MASTER.DATA = data;
		twi_i++;
	} else
		/* transaction finished : send STOP */
		twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
#endif
}

/**
 * \param twi
 * \param freq [kHz]: TWI frequency (usual 100 or 400 kHz)
 * BAUD = (Fosc / (2*ftwi)) - 5
 */
void twi_master_setup(twi_t *twi, uint16_t freq)
{
#if 0
	if (twi == &TWIC)
		twi_ref_on_twic = twi;
	else
		; /* FIXME */

	/* When smart mode is enabled, the acknowledge action, as set by the
	 * ACKACT bit in the CTRLC register, is sent immediately after reading
	 * the DATA register.
	 */
	/* twi->MASTER.CTRLB = TWI_MASTER_SMEN_bm; /\*!< Smart mode */
	twi->MASTER.CTRLA = TWI_MASTER_INTLVL_LO_gc | TWI_MASTER_RIEN_bm |
			    TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;

	/* indicate the current TWI bus state */
	twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
#if F_CPU == 32000000UL
	/* (32000 / (2 * freq)) - 5; // 155 to 100kHz - 35 to 400kHz */
	twi->MASTER.BAUD = 155;
#endif
#endif
}

/**
 *
 */
void twi_write(twi_t *twi, uint8_t slave_address, uint8_t *write_data,
	       uint8_t nb_bytes_to_write)
{
#if 0
	twi_i = 0;
	twi_nb_bytes_to_write = nb_bytes_to_write;
	twi_write_data =  write_data;
	twi->MASTER.ADDR = slave_address << 1;

#if 0
	while (!(twi->MASTER.STATUS & TWI_MASTER_WIF_bm))
		; /* wait write interrupt flag */

	/* if slave returned NACK or did not reply at all : send address
	 * until slave returns ACK
	 */

	for (uint8_t i = 0; i < nb_bytes_to_write; i++) {
		twi->MASTER.DATA = write_data[i]; /* write date and time */
		while (!(twi->MASTER.STATUS & TWI_MASTER_WIF_bm))
			; /* wait write interrupt flag */
	}

	/* send stop */
	twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
#endif
#endif
}

/**
 *
 */
void twi_read(twi_t *twi, uint8_t slave_address, uint8_t *write_data,
	      uint8_t *read_data, uint8_t nb_byte_to_read)
{
#if 0
	uint8_t i;

	twi->MASTER.ADDR = slave_address << 1;
	while (!(twi->MASTER.STATUS & TWI_MASTER_WIF_bm))
		; /* wait write interrupt flag */

	twi->MASTER.DATA = write_data[0];
	while (!(twi->MASTER.STATUS & TWI_MASTER_WIF_bm))
		; /* wait write interrupt flag */

	twi->MASTER.ADDR = (slave_address << 1) | 0x01; /* send read command */
	_delay_ms(1);
	/* while (!(twi->MASTER.STATUS & TWI_MASTER_WIF_bm)) */
		; /* wait write interrupt flag */

	for (i = 0; i < nb_byte_to_read; i++) {
		read_data[i] = twi->MASTER.DATA;
		/* while (!(twi->MASTER.STATUS & TWI_MASTER_RIF_bm)) */
		_delay_ms(1);
		; /* wait read interrupt flag */
	}

	/* send stop */
	/*  while (!(twi->MASTER.STATUS & TWI_MASTER_WIF_bm)) */
	_delay_ms(1);
	; /* wait read interrupt flag */
	twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
#endif
}
