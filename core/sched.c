#include <avr/sleep.h>

#include "irq.h"
#include "kos.h"
#include "log.h"
#include "sched.h"

static timer_t *sched_timer;

static uint32_t ticks = 0;
static volatile uint32_t global_time;

/* Timer Overflow interrupt */
static void irq_timer_tick_handler(void)
{
	ticks++;
	global_time  = (uint32_t)ticks * 20;
	global_time += (uint32_t)(timer_get_cnt(sched_timer)) * 200 / 625;

	kos_schedule();
}

void sched_init(uint16_t period_ms, timer_t *clksrc)
{
	if (period_ms != 20)
		print_err("Not supported\n");

	sched_timer = clksrc;

	/* TCC0 ClkIn == ClkPer / 1024 == 31.25 KHz */
	/* Counter set to 625 for 50Hz output (20ms) */
	timer_normal_mode_setup(clksrc, 625, TC_CLKSEL_DIV1024_gc,
				irq_timer_tick_handler);

	/* */
}

