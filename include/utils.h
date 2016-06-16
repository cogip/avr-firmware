/*
 * utils.h
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <avr/io.h>
#include "odometry.h"

#define WHEELS_DISTANCE		2960 /* pulses soit 279 mm */
#define WHEELS_DIAMETER		60   /* 60 mm - perimeter : 188 mm */
#define ENCODER_RESOLUTION	2000 /* 500*4 pulse per rotate - 10.61 pulse/mm */

/* 2960*pi/2 = 4650 pulses pour 90° soit 51.66 pulses/° */
#define PULSE_PER_DEGREE	51.66
#define PULSE_PER_MM		10.61

#define MAX_ACC			4

typedef struct
{
  pose_t p;
  void * action_function;
  uint8_t status;
  uint8_t can_retro;
} action;

extern uint8_t pose_reached;

extern uint8_t flag_tower_down;

#endif /* UTILS_H_ */
