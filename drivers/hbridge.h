#ifndef HBRIDGE_H_
#define HBRIDGE_H_

#include <avr/io.h>
#include <xmega/timer.h>

typedef struct {
	PORT_t *direction_pin_port;
	uint8_t direction_pin_id;
	uint8_t pwm_channel;
} engine_t;

typedef struct {
	union {
		volatile TC0_t *tc0;
		volatile TC1_t *tc1;
	};
	uint8_t period;
	TC_CLKSEL_t prescaler;

	PORT_t *pwm_port;

	uint8_t engine_nb;
	engine_t engines[];
} hbridge_t;

void hbridge_engine_update(hbridge_t *b, engine_t *e, int16_t pwm);
void hbridge_setup(hbridge_t *b);

#endif /* HBRIDGE_H_ */
