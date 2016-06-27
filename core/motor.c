/*
 * motor.c
 *
 *  Created on: 25 avr. 2015
 *      Author: ldo
 */


#include "hbridge.h"
#include "motor.h"
#include "platform.h"

void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	hbridge_engine_update(&hbridges, &hbridges.engines[HBRIDGE_MOTOR_RIGHT], right_command);
	hbridge_engine_update(&hbridges, &hbridges.engines[HBRIDGE_MOTOR_LEFT],  left_command);
}

void motor_setup()
{
	hbridge_setup(&hbridges);
}
