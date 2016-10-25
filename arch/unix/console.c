#include "console.h"

#include <stdio.h>

//static int uart_putchar(char c, FILE *stream);
//static int uart_getchar(FILE *stream);

//static FILE uart_fdstream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,
//					      _FDEV_SETUP_RW);
//static putchar_cb_t putchar_cb;
//static getchar_cb_t getchar_cb;


//static int uart_putchar(char c, FILE *stream)
//{
//	if (putchar_cb) {
//		if (c == '\n')
//			uart_putchar('\r', stream);
//
//		putchar_cb(c);
//	}
//	return 0;
//}

//static int uart_getchar(FILE *stream)
//{
//	if (getchar_cb) {
//		return getchar_cb();
//	}
//
//	return -1;
//}

void console_init(putchar_cb_t put_cb, getchar_cb_t get_cb)
{
	//putchar_cb = put_cb;
	//getchar_cb = get_cb;

	//stdout = &uart_fdstream;
	//stdin = &uart_fdstream;
}
