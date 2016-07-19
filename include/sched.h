#ifndef SCHED_H_
#define SCHED_H_

#include <stdint.h>

#include "timer.h"

typedef enum {
	TASK_SLEEP,	/* Deep sleep task */
	TASK_READY,	/* Sleep until next tick */
	TASK_RUNNING,	/* Active task */
} task_state_t;

typedef struct {
	task_state_t state;

	/** @retval: updated state of the task */
	task_state_t (*entry_point)(void);
} task_t;


void sched_init(uint16_t period_ms, timer_t *clksrc);
void sched_set_tasks(task_t *tasks_list, uint16_t nb);

void sched_enter_mainloop();

#endif /* SCHED_H_ */
