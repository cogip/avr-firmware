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
void usart_setup(usart_t *usart, uint32_t baudrate)
{
#if F_CPU == 32000000UL
	/* Clock generation : BSEL = fOSC / (2^BSCALE * 16 * fBAUD) - 1 */
	switch (baudrate) {
	case 9600:
		/* 32000000/(2^2 * 16 * 9600) - 1 = 12 */
		usart->BAUDCTRLA = 12;
		usart->BAUDCTRLB = 4 << 4;
		break;
	case 38400:
		/* 32000000/(2^2 * 16 * 38400) - 1 = 12 */
		usart->BAUDCTRLA = 12;
		usart->BAUDCTRLB = 2 << 4;
		break;
	case 115200:
		/* 32000000/(2^0 * 16 * 115200) - 1 = 17 */
		usart->BAUDCTRLA = 17;
		usart->BAUDCTRLB = 0;
		break;
	default:
		return;
	}
#else
#error "MCU frequency not supported"
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

inline int usart_is_data_arrived(usart_t *usart)
{
	return (usart->STATUS & USART_RXCIF_bm);
}
