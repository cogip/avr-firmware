#include <stdio.h>

#include "console.h"
#include "usart.h"
#include "kos.h"

static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);

static FILE uart_fdstream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,
					      _FDEV_SETUP_RW);


static console_t *main_console = NULL;

static int uart_putchar(char c, FILE *stream)
{
	if (main_console) {
		if (c == '\n')
			uart_putchar('\r', stream);

		usart_send(main_console->usart, c);
		return 1;
	}
	return 0;
}

static int uart_getchar(FILE *stream)
{
	if (main_console) {
		while (!usart_is_data_arrived(main_console->usart))
			kos_yield();

		return usart_recv(main_console->usart);
	}

	return -1;
}

int cons_getchar()
{
	return getchar();
}

int cons_is_data_arrived()
{
	return main_console ? usart_is_data_arrived(main_console->usart) : -1;
}

void console_init(console_t *con)
{
	usart_setup(con->usart, con->speed);

	if (!main_console) {
		main_console = con;

		stdout = &uart_fdstream;
		stdin = &uart_fdstream;
	}
}
