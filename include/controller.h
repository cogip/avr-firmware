/*
 * controller.h
 *
 *  Created on: 26 avr. 2015
 *      Author: ldo
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "utils.h"

void controller_setup(void);

polar_t speed_controller(polar_t speed_setpoint, polar_t real_speed);

polar_t controller_update(pose_t pose_setpoint, pose_t current_pose,
			  polar_t speed_setpoint, polar_t current_speed);

#endif /* CONTROLLER_H_ */
