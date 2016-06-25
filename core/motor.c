/*
 * motor.c
 *
 *  Created on: 25 avr. 2015
 *      Author: ldo
 */

#include <math.h>
#include <xmega/timer.h>

#include "action.h"
#include "motor.h"
#include "route.h"

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

/**
 * \param pwm value for motor (signed value)
 */
static void left_motor_drive(int16_t pwm)
{
	/* limitation de la commande de vitesse */
	uint8_t pwm_limit = pwm_limitation(pwm, MAX_PWM);

	/* advance */
	if (pwm > 0)
		PORTD.OUTSET = PIN4_bm;
	else
		PORTD.OUTCLR = PIN4_bm;

	/* generate PWM */
	timer_0_pwm_duty_cycle(&TCE0, 0, pwm_limit);
}

/**
 * \param pwm value for motor (signed value)
 */
static void right_motor_drive(int16_t pwm)
{
	/* limitation de la commande de vitesse */
	uint8_t pwm_limit = pwm_limitation(pwm, MAX_PWM);

	/* advance */
	if (pwm > 0)
		PORTD.OUTCLR = PIN5_bm;
	else
		PORTD.OUTSET = PIN5_bm;

	/* generate PWM */
	timer_0_pwm_duty_cycle(&TCE0, 1, pwm_limit);
}

void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	right_motor_drive(right_command);
	left_motor_drive(left_command);
}

