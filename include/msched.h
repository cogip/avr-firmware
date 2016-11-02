#ifndef MSCHED_H_
#define MSCHED_H_

#include <stdint.h>

#include "hwtimer.h"

void msched_init(uint16_t period_ms, hwtimer_t *clksrc);
uint16_t msched_get_tickms(void);

#endif /* MSCHED_H_ */
