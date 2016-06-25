#include <xmega/timer.h>

#include "hbridge.h"

#define MAX_PWM		(TCE0_MOTOR_PER_VALUE)

/** limite la commande de vitesse
 * @param value from -16535 to 16535
 * @return pwm value from 0 to max
 */
static uint8_t pwm_limitation(int16_t value, uint8_t max)
{
	int16_t	out = value > 0 ? value : -value;

	return out > max ? max : (uint8_t) out;
}

void hbridge_update(hbridge_t *b, int16_t pwm)
{
	/* limitation de la commande de vitesse */
	uint8_t pwm_period = pwm_limitation(pwm, MAX_PWM);

	if (pwm > 0)
		/* forward direction */
		b->direction_pin_port->OUTSET = b->direction_pin_id;
	else
		/* backward direction */
		b->direction_pin_port->OUTCLR = b->direction_pin_id;

	/* generate PWM */
	timer_0_pwm_duty_cycle(&TCE0, b->pwm_channel, pwm_period);
}

