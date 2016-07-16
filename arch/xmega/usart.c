/**
 * \file usart.c
 *
 * \date 22 nov. 2014
 * \author ldo
 */

#include "usart.h"

/**
 * set the TDX and RDX pin
 * set the baudrate and frame format
 * set the mode of operation
 * enable the transmitter and/or the receiver
 *
 * internal clock generation to Asynchronous normal speed mode
 *
 * ATXMEGA128A1 use PORTC, PORTD, PORTE or PORTF. Each port has one USART0 and
 * one USART1.
 * Notation USARTC0, USARTC1, USARTD0, USARTD1, USARTE0, USARTE1, USARTF0 and
 * USARTF1.
 */
void usart_setup(usart_t *usart)
{
	/* Clock generation : BSEL = fOSC / (2^BSCALE * 16 * fBAUD) - 1 */
	/* 32000000/(2^0 * 16 * 9600) - 1 = 207 */
	/* 32000000/(2^0 * 16 * 115200) - 1 = 17 */
#if F_CPU == 32000000UL
	usart->BAUDCTRLA = 207; /* 12; //51; */
	usart->BAUDCTRLA = 17; /* 12; //51; */
	usart->BAUDCTRLB = 0;
#endif

	/* Frame format : 8-data bits, no parity, 1 stop bit. */
	usart->CTRLC = (USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc);

	/* mode operation : asynchronous mode */
	usart->CTRLC |= USART_CMODE_ASYNCHRONOUS_gc;

	/* Enable both RX and TX. */
	usart->CTRLB = USART_TXEN_bm | USART_RXEN_bm;
}

/* transmit a char */
void usart_send(usart_t *usart, uint8_t data)
{
	/* Wait for last character */
	while (!(usart->STATUS & USART_DREIF_bm))
		;

	/* Output 1 character */
	usart->DATA = data;
}

int usart_recv(usart_t *usart)
{
	/* Wait for a character */
	while (!(usart->STATUS & USART_RXCIF_bm))
		;

	/* Output 1 character */
	return usart->DATA;
}
