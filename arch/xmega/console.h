#ifndef CONSOLE_H
#define CONSOLE_H

typedef void (*putchar_cb_t)(const char);
typedef int (*getchar_cb_t)(void);

void console_init(putchar_cb_t, getchar_cb_t);

#endif /* CONSOLE_H */
