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
	timer_pwm_duty_cycle(b->tc, e->pwm_channel, pwm_period);
}

void hbridge_setup(hbridge_t *b)
{
	uint8_t i;

	/* Configure PWM pin as output */
	for (i = 0; i < b->engine_nb; i++)
		b->pwm_port->DIRSET = (1 << b->engines[i].pwm_channel);

	/* setup frequency waveform generation (PWM) */
	timer_pwm_mode_setup(b->tc, b->period, b->prescaler);

	for (i = 0; i < b->engine_nb; i++)
		timer_pwm_enable(b->tc, b->engines[i].pwm_channel);
}
