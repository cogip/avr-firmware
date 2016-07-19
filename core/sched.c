#include <avr/sleep.h>

#include "irq.h"
#include "log.h"
#include "sched.h"

static timer_t *sched_timer;

static uint32_t ticks = 0;
static volatile uint32_t global_time;

static task_t *tasks;
static uint16_t tasks_nb;


/* Timer Overflow interrupt */
static void irq_timer_tick_handler(void)
{
	uint16_t i;

	ticks++;
	global_time  = (uint32_t)ticks * 20;
	global_time += (uint32_t)(timer_get_cnt(sched_timer)) * 200 / 625;

	//irq_disable();
	for (i = 0; i < tasks_nb; i++)
		tasks[i].state = TASK_RUNNING;
	//irq_enable();
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


void sched_set_tasks(task_t *tasks_list, uint16_t nb)
{
	irq_disable();
	tasks = tasks_list;
	tasks_nb = nb;
	irq_enable();
}

void sched_enter_mainloop()
{
	set_sleep_mode(SLEEP_MODE_IDLE);

	for (;;) {
		uint16_t i;
		uint8_t tasks_all_sleep = TRUE;

		irq_disable();
		/* Execute all user tasks */
		for (i = 0; i < tasks_nb; i++)
		{
			task_t *current = &tasks[i];

			if ((current->state == TASK_READY ||
			     current->state == TASK_RUNNING) &&
			    current->entry_point)
			{
				task_state_t next_state;

				irq_enable();
				next_state = current->entry_point();
				irq_disable();

				if (next_state == TASK_RUNNING)
					tasks_all_sleep = FALSE;

				current->state = next_state;
			}
		}
		irq_enable();

		/* Enter sleep mode if possible */
		if (tasks_all_sleep) {
			sleep_enable();
			sleep_cpu();
			sleep_disable();
		}
	}
}
