#include <math.h>

#include "odometry.h"
#include "platform.h"

#include "controller.h"

/**
 * \fn polar_t compute_error(const pose_t p1, const pose_t p2)
 * \brief compute error between 2 poses
 * \param p1 : setpoint pose
 * \param p2 : measure pose
 * \return distance and angle errors between 2 poses
 */
static polar_t compute_error(controller_t *ctrl,
			     const pose_t p1, const pose_t p2)
{
	polar_t error;

	double x = p1.x - p2.x;
	double y = p1.y - p2.y;
	double O = atan2(y, x);

	error.angle = O * ctrl->wheels_distance;
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
polar_t speed_controller(controller_t *ctrl,
			 polar_t speed_order, polar_t speed_current)
{
	polar_t speed_error;
	polar_t command;

	speed_error.distance = speed_order.distance - speed_current.distance;
	speed_error.angle = speed_order.angle - speed_current.angle;

	command.distance = pid_controller(&ctrl->linear_speed_pid,
					  speed_error.distance);
	command.angle = pid_controller(&ctrl->angular_speed_pid,
				       speed_error.angle);

	return command;
}

polar_t controller_update(controller_t *ctrl,
			  pose_t pose_order,
			  pose_t pose_current,
			  polar_t speed_order,
			  polar_t speed_current)
{
	/* ******************** position pid controller ******************** */

	/* compute position error */
	polar_t position_error = compute_error(ctrl, pose_order, pose_current);

	ctrl->pose_reached = 0;

	/* position correction */
	if (position_error.distance > ctrl->min_distance_for_angular_switch) {
		position_error.angle -= pose_current.O; /* [pulse] */

		if (fabs(position_error.angle) > (M_PI * ctrl->wheels_distance / 2.0)) {
			position_error.distance = -position_error.distance;

			if (position_error.angle < 0)
				position_error.angle += M_PI * ctrl->wheels_distance;
			else
				position_error.angle -= M_PI * ctrl->wheels_distance;
		}
	} else {
		/* orientation correction (position is reached) */
		position_error.distance = 0;
		pid_reset(&ctrl->linear_pose_pid);

		/* unit in [pulse] */
		position_error.angle = pose_order.O - pose_current.O;

		/* orientation is reached */
		if (fabs(position_error.angle) < ctrl->min_angle_for_pose_reached) {
			position_error.angle = 0;
			pid_reset(&ctrl->angular_pose_pid);
			ctrl->pose_reached = 1;
		}
	}

	/* compute speed command with position pid controller */
	polar_t command;

	command.distance = pid_controller(&ctrl->linear_pose_pid,
					  position_error.distance);
	command.angle = pid_controller(&ctrl->angular_pose_pid,
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
	return speed_controller(ctrl, speed, speed_current);
}

inline uint8_t controller_get_pose_reached(controller_t *ctrl)
{
	return ctrl->pose_reached;
}
