/**
 * \file usart.h
 *
 * \date 23 nov. 2014
 *  \author ldo
 */

#ifndef USART_H_
#define USART_H_

#include <avr/io.h>

typedef USART_t usart_t;

/**
 * \fn
 * \brief setup usart 9600 baud, 8-data bits, no parity, 1 stop bit
 */
void usart_setup(usart_t *usart);

void usart_send(usart_t *usart, uint8_t data);
int usart_recv(usart_t *usart);

int usart_is_data_arrived(usart_t *usart);

#endif /* USART_H_ */
