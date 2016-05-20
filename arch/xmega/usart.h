/**
 * \file xmega_usart.h
 *
 * \date 23 nov. 2014
 *  \author ldo
 */

#ifndef XMEGA_USART_H_
#define XMEGA_USART_H_

#include <avr/io.h>

/**
 * \fn
 * \brief setup usart 9600 baud, 8-data bits, no parity, 1 stop bit
 */
void
xmega_usart_setup (USART_t *usart);

// transmit a char
void
xmega_usart_transmit (USART_t *usart, uint8_t data);

#endif /* XMEGA_USART_H_ */
