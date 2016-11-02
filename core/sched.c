#include "irq.h"
#include "kos.h"
#include "log.h"
#include "sched.h"

static uint16_t sched_ms;
static hwtimer_t *sched_timer;

static uint32_t ticks = 0;

/* Timer Overflow interrupt */
static void irq_timer_tick_handler(void)
{
	ticks++;

	kos_tick_schedule();
}

void sched_init(uint16_t period_ms, hwtimer_t *clksrc)
{
	/* TCC0 ClkIn == ClkPer / 1024 == 31.25 KHz */
	switch (period_ms) {
	case 5:
		/* Counter set to 156.25 for 200Hz output (5ms) */
		/* Value tuned with scope : 157 */
		timer_normal_mode_setup(clksrc, 157, TC_CLKSEL_DIV1024_gc,
					irq_timer_tick_handler);
		break;
	case 10:
		/* Counter set to 312.5 for 100Hz output (10ms) */
		/* Value tuned with scope : 325 */
		timer_normal_mode_setup(clksrc, 325, TC_CLKSEL_DIV1024_gc,
					irq_timer_tick_handler);
		break;
	case 20:
		/* Counter set to 625 for 50Hz output (20ms) */
		/* Value tuned with scope : 645 */
		timer_normal_mode_setup(clksrc, 645, TC_CLKSEL_DIV1024_gc,
					irq_timer_tick_handler);
		break;
	default:
		print_err("Not supported\n");
		return;
	}

	sched_timer = clksrc;
	sched_ms = period_ms;
}

inline uint16_t sched_get_tickms(void)
{
	return sched_ms;
}

