#include "hbridge.h"

/** limits speed command
 * @param value from -16535 to 16535
 * @return pwm value from 0 to max
 */
static uint8_t pwm_limitation(int16_t value, uint8_t max)
{
	int16_t	out = value > 0 ? value : -value;

	return out > max ? max : (uint8_t) out;
}

void hbridge_engine_update(hbridge_t *b, uint8_t engine_idx, int16_t pwm)
{
	/* limits speed command */
	uint8_t pwm_period = pwm_limitation(pwm, b->period);
	engine_t *e = &b->engines[engine_idx];

	/* signed of pwm value is applied on direction gpio */
	gpio_set_output(e->direction_pin_port, e->direction_pin_id, pwm > 0);

	/* generate PWM */
	timer_pwm_duty_cycle(b->tc, e->pwm_channel, pwm_period);
}

void hbridge_setup(hbridge_t *b)
{
	uint8_t i;

	for (i = 0; i < b->engine_nb; i++) {
		engine_t *e = &b->engines[i];

		/* Configure PWM pin as output */
		gpio_set_direction(b->pwm_port,
				   e->pwm_channel,
				   GPIO_DIR_OUT);

		/* Configure direction pin as output */
		gpio_set_direction(e->direction_pin_port,
				   e->direction_pin_id,
				   GPIO_DIR_OUT);
	}

	/* setup frequency waveform generation (PWM) */
	timer_pwm_mode_setup(b->tc, b->period, b->prescaler);

	for (i = 0; i < b->engine_nb; i++)
		timer_pwm_enable(b->tc, b->engines[i].pwm_channel);
}
