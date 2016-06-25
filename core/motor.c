/*
 * motor.c
 *
 *  Created on: 25 avr. 2015
 *      Author: ldo
 */


#include "hbridge.h"
#include "motor.h"

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

void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	hbridge_update(&motors[HBRIDGE_MOTOR_RIGHT], right_command);
	hbridge_update(&motors[HBRIDGE_MOTOR_LEFT],  left_command);
}

