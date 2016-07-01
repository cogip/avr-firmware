#ifndef QDEC_H_
#define QDEC_H_

#include <avr/io.h>
#include <stdint.h>
#include "timer.h"

typedef struct {
	PORT_t *pin_port;
	uint8_t pin_qdph0;
	uint8_t pin_qdph90;
	TC_EVSEL_t event_channel;
	timer_t *tc;
	uint16_t line_count;
	int8_t polarity; /* -1 to negate, 1 otherwise */
} qdec_t;

int8_t qdec_setup(qdec_t *qdec);
int16_t qdec_read(qdec_t *qdec);

#endif /* QDEC_H_ */
