#ifndef USART_H_
#define USART_H_

#include <stdint.h>
#if defined(__AVR__)
#include <avr/io.h>

typedef USART_t usart_t;
#else
typedef void usart_t;

char _usart_[4];
#define USARTC0 (_usart_[0])
#define USARTD0 (_usart_[1])
#endif

/**
 * \fn
 * \brief setup usart. tunable baudrate, 8-data bits, no parity, 1 stop bit
 */
void usart_setup(usart_t *usart, uint32_t baudrate);

void usart_send(usart_t *usart, uint8_t data);
int usart_recv(usart_t *usart);

int usart_is_data_arrived(usart_t *usart);

#endif /* USART_H_ */
