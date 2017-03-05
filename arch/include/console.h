#ifndef CONSOLE_H
#define CONSOLE_H

#include "usart.h"

typedef struct {
	usart_t *usart;
	uint32_t speed;
} console_t;

#define printf_ratelimited(...) \
do { \
	static uint8_t _cpt = (25); \
\
	if (! --_cpt) { \
		_cpt = (25); \
		printf(__VA_ARGS__); \
	} \
} while(0)

#define cons_printf printf
#define cons_scanf scanf

int cons_getchar();

void console_init(console_t *con);

#endif /* CONSOLE_H */
