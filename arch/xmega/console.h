#ifndef CONSOLE_H
#define CONSOLE_H

typedef void (*putchar_cb_t)(const char);
typedef int (*getchar_cb_t)(void);

#define printf_ratelimited(...) \
do { \
	static uint8_t _cpt = (25); \
\
	if (! --_cpt) { \
		_cpt = (25); \
		printf(__VA_ARGS__); \
	} \
} while(0)

void console_init(putchar_cb_t, getchar_cb_t);

#endif /* CONSOLE_H */
