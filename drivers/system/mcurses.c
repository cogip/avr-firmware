#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "irq.h"
#include "mcurses.h"

/* Scrolling window configuration: below is the number of fixed lines */
#define NB_FIXED_LINES		4

/* Top area organized by fields: below is their width in nb of characters */
#define FIELDS_WIDTH		40

void mcurses_getyx(uint8_t *y, uint8_t *x)
{
	char c;
	char buf[10] = {'0',};
	uint8_t bufidx = 0;

	irq_disable();

	cons_printf("\033[6n");

	/*
	 * Cursor position raport  (Response to request cursor position)
	 *   Esc  [ <n1> ; <n2> R
	 *   033 133    073    122
	 */
	do {
		c = getchar();
	} while (c != '\033');

	do {
		c = getchar();
	} while (c != '[');

	for (bufidx = 0; buf[bufidx-1] != ';'; bufidx++)
		buf[bufidx] = getchar();
	buf[--bufidx] = '\0';
	*y = atoi(buf) - 1;

	for (bufidx = 0; buf[bufidx-1] != 'R'; bufidx++)
		buf[bufidx] = getchar();
	buf[--bufidx] = '\0';
	*x = atoi(buf) - 1;

	irq_enable();
}

void mcurses_getmaxyx(uint8_t *y, uint8_t *x)
{
	uint8_t origy, origx;

	mcurses_getyx(&origy, &origx);

	mcurses_gotoyx(255, 255);
	mcurses_getyx(y, x);

	mcurses_gotoyx(origy, origx);
}

void mcurses_gotoyx(uint8_t y, uint8_t x)
{
	irq_disable();
	cons_printf("\033[%d;%dH", y + 1, x + 1);
	irq_enable();
}

void mcurses_clear()
{
	irq_disable();
	cons_printf("\033[2J");
	irq_enable();
}

void mcurses_init()
{
	uint8_t maxy, maxx;

	mcurses_clear();
	mcurses_gotoyx(0, 0);

	/* Set scrolling fast mode */
	irq_disable();
	cons_printf("\033[?4l");
	irq_enable();

	/* Set scrolling window */
	/* Top lines are out of scrolling zone (var. monitoring area) */
	/* keep NB_FIXED_LINES lines + 1 for hbar */
	mcurses_getmaxyx(&maxy, &maxx);
	irq_disable();
	cons_printf("\033[%d;%dr", NB_FIXED_LINES + 1 + 1, maxy);
	irq_enable();

	/* Draw hbar */
	mcurses_gotoyx(NB_FIXED_LINES, 0);
	while (maxx--)
		cons_printf("-");

	mcurses_gotoyx(NB_FIXED_LINES + 1, 0);
}

static void mcurses_monitor_printf_v(uint8_t idx, const char *format, va_list args)
{
	static char buf[FIELDS_WIDTH] = {0,};
	uint8_t y, x;

	/* save context */
	mcurses_getyx(&y, &x);

	/* write title on top right */
	mcurses_gotoyx(idx % NB_FIXED_LINES,
		       FIELDS_WIDTH * (idx / NB_FIXED_LINES));

	vsnprintf(buf, FIELDS_WIDTH, format, args);
	cons_printf("%s", buf);

	/* restore context */
	mcurses_gotoyx(y, x);
}

void mcurses_monitor_printf(uint8_t idx, const char *format, ...)
{
	va_list args;

	va_start (args, format);
	mcurses_monitor_printf_v(idx, format, args);
	va_end (args);
}
