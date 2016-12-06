#ifndef MCURSES_H
#define MCURSES_H

#include <stdint.h>

void mcurses_getyx(uint8_t *y, uint8_t *x);
void mcurses_getmaxyx(uint8_t *y, uint8_t *x);

void mcurses_gotoyx(uint8_t y, uint8_t x);

void mcurses_clear();
void mcurses_init();

void mcurses_monitor_printf(uint8_t idx, const char *format, ...);

#endif /* MCURSES_H */
