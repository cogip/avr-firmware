/*
 * controller.c
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#include <math.h>

#include "odometry.h"
#include "pid.h"
#include "platform.h"

#include "controller.h"

PID_t linear_speed_pid, angular_speed_pid;
PID_t linear_pose_pid, angular_pose_pid;

static uint8_t pose_reached;
static double wheels_distance; /*!< robot wheels distance [pulse] */

/**
 * TODO reglage des coeffs PID position
 * speed PI controller
 */
void controller_setup(double d)
{
	wheels_distance = d;

	pid_setup(&linear_speed_pid, 1.5, 0.2, 0);
	pid_setup(&angular_speed_pid, 1.5, 0.2, 0);
	pid_setup(&linear_pose_pid, 1, 0, 20);
	pid_setup(&angular_pose_pid, 1, 0, 20);
}

/**
 * \fn polar_t compute_error(const pose_t p1, const pose_t p2)
 * \brief compute error between 2 poses
 * \param p1 : setpoint pose
 * \param p2 : measure pose
 * \return distance and angle errors between 2 poses
 */
static polar_t compute_error(const pose_t p1, const pose_t p2)
{
	polar_t error;

	double x = p1.x - p2.x;
	double y = p1.y - p2.y;
	double O = atan2(y, x);

	error.angle = O * wheels_distance;
	error.distance = sqrt(square(x) + square(y));

	return error;
}

/**
 * \fn limit_speed_command
 * \brief limit speed command to maximum acceleration and speed setpoint
 * \param command : computed speed by position PID controller
 * \param final_speed : maximum speed
 * \param real_speed
 * \return speed_order
 */
static double limit_speed_command(double command,
				  double final_speed,
				  double real_speed)
{
	/* limit speed command (maximum acceleration) */
	int16_t a = command - real_speed;

	if (a > MAX_ACC)
		command = real_speed + MAX_ACC;

	if (a < -MAX_ACC)
		command = real_speed - MAX_ACC;

	/* limit speed command (speed setpoint) */
	if (command > final_speed)
		command = final_speed;

	if (command < -final_speed)
		command = -final_speed;

	return command;
}

/**
 *
 */
polar_t speed_controller(polar_t speed_order, polar_t speed_current)
{
	polar_t speed_error;
	polar_t command;

	speed_error.distance = speed_order.distance - speed_current.distance;
	speed_error.angle = speed_order.angle - speed_current.angle;

	command.distance = pid_controller(&linear_speed_pid,
					  speed_error.distance);
	command.angle = pid_controller(&angular_speed_pid,
				       speed_error.angle);

	return command;
}

polar_t controller_update(pose_t pose_order,
			  pose_t pose_current,
			  polar_t speed_order,
			  polar_t speed_current)
{
	/* ******************** position pid controller ******************** */

	/* compute position error */
	polar_t position_error = compute_error(pose_order, pose_current);

	pose_reached = 0;

	/* position correction */
	if (position_error.distance > 500) {
		position_error.angle -= pose_current.O; /* [pulse] */

		if (fabs(position_error.angle) > (M_PI * wheels_distance / 2.0)) {
			position_error.distance = -position_error.distance;

			if (position_error.angle < 0)
				position_error.angle += M_PI * wheels_distance;
			else
				position_error.angle -= M_PI * wheels_distance;
		}
	} else {
		/* orientation correction (position is reached) */
		position_error.distance = 0;
		pid_reset(&linear_pose_pid);

		/* unit in [pulse] */
		position_error.angle = pose_order.O - pose_current.O;

		/* orientation is reached */
		if (fabs(position_error.angle) < 100) {
			position_error.angle = 0;
			pid_reset(&angular_pose_pid);
			pose_reached = 1;
		}
	}

	/* compute speed command with position pid controller */
	polar_t command;

	command.distance = pid_controller(&linear_pose_pid,
					  position_error.distance);
	command.angle = pid_controller(&angular_pose_pid,
				       position_error.angle);

	/* limit speed command */
	polar_t speed;

	speed.distance = limit_speed_command(command.distance,
					     speed_order.distance,
					     speed_current.distance);
	speed.angle = limit_speed_command(command.angle,
					  speed_order.angle,
					  speed_current.angle);

	/* ********************** speed pid controller ********************* */
	return speed_controller(speed, speed_current);
}

inline uint8_t controller_get_pose_reached()
{
	return pose_reached;
}
