#include <xmega/timer.h>

#include "hbridge.h"

/** limite la commande de vitesse
 * @param value from -16535 to 16535
 * @return pwm value from 0 to max
 */
static uint8_t pwm_limitation(int16_t value, uint8_t max)
{
	int16_t	out = value > 0 ? value : -value;

	return out > max ? max : (uint8_t) out;
}

void hbridge_engine_update(hbridge_t *b, engine_t *e, int16_t pwm)
{
	/* limitation de la commande de vitesse */
	uint8_t pwm_period = pwm_limitation(pwm, b->period);

	if (pwm > 0)
		/* forward direction */
		e->direction_pin_port->OUTSET = e->direction_pin_id;
	else
		/* backward direction */
		e->direction_pin_port->OUTCLR = e->direction_pin_id;

	/* generate PWM */
	if (IS_TIMER0(b->tc0))
		timer_0_pwm_duty_cycle(b->tc0, e->pwm_channel, pwm_period);
	else
		; /* timer_1: pwm mode not implemented */
}

void hbridge_setup(hbridge_t *b)
{
	uint8_t i;

	/* Configure PWM pin as output */
	for (i = 0; i < b->engine_nb; i++)
		b->pwm_port->DIRSET = (1 << b->engines[i].pwm_channel);

	/* setup frequency waveform generation (PWM) */
	if (IS_TIMER0(b->tc0)) {
		timer_0_pwm_mode_setup(b->tc0, b->period, b->prescaler);

		for (i = 0; i < b->engine_nb; i++)
			timer_0_pwm_enable(b->tc0, b->engines[i].pwm_channel);
	} else if (IS_TIMER1(b->tc1))
		; /* timer_1: pwm mode not implemented */
}
