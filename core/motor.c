/*
 * motor.c
 *
 *  Created on: 25 avr. 2015
 *      Author: ldo
 */

#include <math.h>
#include <xmega/timer.h>

#include "motor.h"

#define MAX_PWM		(TCE0_MOTOR_PER_VALUE)

typedef struct {
	PORT_t *direction_pin_port;
	uint8_t direction_pin_id;
	uint8_t pwm_channel;
} hbridge_t;

#define HBRIDGE_MOTOR_LEFT		0
#define HBRIDGE_MOTOR_RIGHT		1

hbridge_t motors[] = {
	{
		/* left motor */
		.direction_pin_port = &PORTD,
		.direction_pin_id = PIN4_bm,
		.pwm_channel = 0,
	},
	{
		/* right motor */
		.direction_pin_port = &PORTD,
		.direction_pin_id = PIN5_bm,
		.pwm_channel = 1,
	},
};

/** limite la commande de vitesse
 * @param value from -16535 to 16535
 * @return pwm value from 0 to max
 */
static uint8_t pwm_limitation(int16_t value, uint8_t max)
{
	int16_t	out = value > 0 ? value : -value;

	return out > max ? max : (uint8_t) out;
}

static void hbridge_update(hbridge_t *b, int16_t pwm)
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

void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	hbridge_update(&motors[HBRIDGE_MOTOR_RIGHT], right_command);
	hbridge_update(&motors[HBRIDGE_MOTOR_LEFT],  left_command);
}

