/*
 * controller.h
 *
 *  Created on: 26 avr. 2015
 *      Author: ldo
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "utils.h"

void
controller_setup (void);

polar
speed_controller (polar speed_setpoint, polar real_speed);

polar
controller_update (pose pose_setpoint, pose current_pose, polar speed_setpoint,
		   polar current_speed);

#endif /* CONTROLLER_H_ */
