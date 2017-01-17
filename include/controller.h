#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <stdint.h>

#include "odometry.h"
#include "pid.h"

typedef enum {
	CTRL_STATE_STOP = 0,
	CTRL_STATE_INGAME,
#if defined(CONFIG_CALIBRATION)
	CTRL_STATE_CALIB_MODE1,
	CTRL_STATE_CALIB_MODE2,
#endif
} controller_mode_t;

typedef struct {
	double wheels_distance;

	PID_t linear_speed_pid;
	PID_t angular_speed_pid;
	PID_t linear_pose_pid;
	PID_t angular_pose_pid;

	/* Distance approximation to switch to angular correction */
	uint16_t min_distance_for_angular_switch;

	/* Angle approximation to switch to position reached state */
	uint16_t min_angle_for_pose_reached;


	/* Dynamics variables */
	controller_mode_t mode;
	uint8_t pose_reached;
} controller_t;


polar_t speed_controller(controller_t *ctrl,
			 polar_t speed_setpoint, polar_t real_speed);

polar_t controller_update(controller_t *ctrl,
			  pose_t pose_setpoint, pose_t current_pose,
			  polar_t speed_setpoint, polar_t current_speed);

uint8_t controller_get_pose_reached(controller_t *ctrl);

void task_controller_update();

#endif /* CONTROLLER_H_ */
