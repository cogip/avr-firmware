/**
 * \file usart.h
 *
 * \date 23 nov. 2014
 *  \author ldo
 */

#ifndef USART_H_
#define USART_H_

#include <avr/io.h>

/**
 * \fn
 * \brief setup usart 9600 baud, 8-data bits, no parity, 1 stop bit
 */
void usart_setup(USART_t *usart);

/* transmit a char */
void usart_send(USART_t *usart, uint8_t data);

#endif /* USART_H_ */
