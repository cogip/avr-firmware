/*
 * motor.h
 *
 *  Created on: 25 avr. 2015
 *      Author: ldo
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "utils.h"

void
motor_drive (polar command);

void
left_motor_drive (int16_t pwm);

void
right_motor_drive (int16_t pwm);

#endif /* MOTOR_H_ */
