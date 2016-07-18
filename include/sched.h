#ifndef SCHED_H_
#define SCHED_H_

#include <stdint.h>

#include "timer.h"

typedef enum {
	TASK_SLEEP,
	TASK_RUNNING,
} task_state_t;

typedef struct {
	task_state_t state;

	/** @retval: TRUE if task require to continue */
	uint8_t (*entry_point)(void);
} task_t;


void sched_init(uint16_t period_ms, timer_t *clksrc);
void sched_set_tasks(task_t *tasks_list, uint16_t nb);

void sched_enter_mainloop();

#endif /* SCHED_H_ */
