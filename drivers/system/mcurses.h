#ifndef MCURSES_H
#define MCURSES_H

#include <stdint.h>

#ifdef CONFIG_MCURSES

void mcurses_getyx(uint8_t *y, uint8_t *x);
void mcurses_getmaxyx(uint8_t *y, uint8_t *x);

void mcurses_gotoyx(uint8_t y, uint8_t x);

void mcurses_clear();
void mcurses_init();

void mcurses_monitor_printf(uint8_t idx, const char *format, ...);

#else

#define mcurses_getyx(y,x)
#define mcurses_getmaxyx(y,x)

#define mcurses_gotoyx(y,x)

#define mcurses_clear()
#define mcurses_init()

#define mcurses_monitor_printf(i, f, ...);

#endif /* CONFIG_MCURSES */

#endif /* MCURSES_H */
