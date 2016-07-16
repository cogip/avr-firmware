#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);
static FILE uart_fdstream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar,
					      _FDEV_SETUP_RW);

static putchar_cb_t putchar_cb;
static getchar_cb_t getchar_cb;

#if 0
/* #ifdef CONFIG_LOGGING_TIME */
#include <sys/time.h>

static void print_logtime(FILE *stream)
{
	struct timeval now;
	static struct timeval first = { 0, 0 };

	gettimeofday(&now, NULL);
	if (!first.tv_sec) {
		first.tv_sec = now.tv_sec;
		first.tv_usec = now.tv_usec;
	}
	if (now.tv_usec < first.tv_usec) {
		now.tv_sec--;
		now.tv_usec += 1000000;
	}
	now.tv_sec -= first.tv_sec;
	now.tv_usec -= first.tv_usec;

	fprintf(stream, "[%d.%06d]",
		(int)now.tv_sec, (int)now.tv_usec);
}
#else
#define print_logtime(stream) do { (void)(stream); } while(0)
#endif

static void print_log_v(int level, const char *function, const char *format,
		va_list args)
{
	FILE *stream = stdout;
	const char *prefix;
	const int dbg = CONFIG_DEFAULT_LOGLEVEL;

	if (level > dbg)
		return;

	switch (level) {
	case LOG_LEVEL_INFO:
		prefix = "<I>";
		break;
	case LOG_LEVEL_WARNING:
		prefix = "<W>";
		break;
	case LOG_LEVEL_ERROR:
		prefix = "<E>";
		break;
	case LOG_LEVEL_DEBUG:
		prefix = "<D>";
		break;
	default:
		prefix = "<U>";
		break;
	}

	fprintf(stream, "%s", prefix);
	print_logtime(stream);
	fprintf(stream, " %s(): ", function);

	vfprintf(stream, format, args);
}

void print_log(int level, const char *function, const char *format, ...)
{
	va_list args;

	va_start (args, format);
	print_log_v(level, function, format, args);
	va_end (args);
}

static int uart_putchar(char c, FILE *stream)
{
	if (putchar_cb) {
		if (c == '\n')
			uart_putchar('\r', stream);

		putchar_cb(c);
	}
	return 0;
}

static int uart_getchar(FILE *stream)
{
	if (getchar_cb) {
		return getchar_cb();
	}

	return -1;
}

void log_init(putchar_cb_t put_cb, getchar_cb_t get_cb)
{
	putchar_cb = put_cb;
	getchar_cb = get_cb;

	stdout = &uart_fdstream;
	stdin = &uart_fdstream;
}
