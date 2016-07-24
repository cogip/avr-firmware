#ifndef SCHED_H_
#define SCHED_H_

#include <stdint.h>

#include "timer.h"

void sched_init(uint16_t period_ms, timer_t *clksrc);
uint16_t sched_get_tickms(void);

#endif /* SCHED_H_ */
