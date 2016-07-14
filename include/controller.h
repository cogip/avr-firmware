/*
 * controller.h
 *
 *  Created on: 26 avr. 2015
 *      Author: ldo
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "odometry.h"

void controller_setup(double);

polar_t speed_controller(polar_t speed_setpoint, polar_t real_speed);

polar_t controller_update(pose_t pose_setpoint, pose_t current_pose,
			  polar_t speed_setpoint, polar_t current_speed);

inline uint8_t controller_get_pose_reached();

#endif /* CONTROLLER_H_ */
