#include <avr/io.h>
#include "platform.h"

/* TCE0 ClkIn == ClkPer / 8 == 4000 KHz */
/* Counter set to 200 for 20KHz output */
#define TCE0_MOTOR_PRESCALER		TC_CLKSEL_DIV8_gc
#define TCE0_MOTOR_PER_VALUE		200

hbridge_t hbridges = {
	.tc0 = &TCE0,
	.period = TCE0_MOTOR_PER_VALUE,
	.prescaler = TCE0_MOTOR_PRESCALER,

	.pwm_port = &PORTE, /* TODO: can be 'guessed' from timer ref above */

	.engine_nb = 2,
	.engines = {
		[HBRIDGE_MOTOR_LEFT] = {
			/* left motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN4_bm,
			.pwm_channel = PIN0_bp,
		},
		[HBRIDGE_MOTOR_RIGHT] = {
			/* right motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN5_bm,
			.pwm_channel = PIN1_bp,
		},
	},
};

