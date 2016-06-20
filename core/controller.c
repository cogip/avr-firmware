/*
 * controller.c
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#include <math.h>

#include "odometry.h"
#include "pid.h"

#include "controller.h"

PID linear_speed_pid, angular_speed_pid;
PID linear_pose_pid, angular_pose_pid;

/**
 * TODO reglage des coeffs PID position
 * speed PI controller
 */
void
controller_setup (void)
{
  pid_setup (&linear_speed_pid, 1.5, 0.2, 0);
  pid_setup (&angular_speed_pid, 1.5, 0.2, 0);
  pid_setup (&linear_pose_pid, 1, 0, 20);
  pid_setup (&angular_pose_pid, 1, 0, 20);
}

/**
 \fn polar_t compute_error (const pose_t p1, const pose_t p2)
 \brief compute error between 2 poses
 \param p1 : setpoint pose
 \param p2 : measure pose
 \return distance and angle errors between 2 poses
 */
polar_t
compute_error (const pose_t p1, const pose_t p2)
{
  polar_t error;
  double x = p1.x - p2.x;
  double y = p1.y - p2.y;
  error.distance = sqrt (square (x) + square (y));

  double O = atan2 (y, x);
  error.angle = O * WHEELS_DISTANCE;

  return error;
}

/**
 * \fn double limit_speed_command (double command, double final_speed, double real_speed)
 * \brief limit speed command to maximum acceleration and speed setpoint
 * \param command : computed speed by position PID controller
 * \param final_speed : maximum speed
 * \param real_speed
 * \return speed_setpoint
 * */
double
limit_speed_command (double command, double final_speed, double real_speed)
{
  /* limit speed command (maximum acceleration) */
  int16_t a = command - real_speed;
  if (a > MAX_ACC)
    {
      command = real_speed + MAX_ACC;
    }
  if (a < -MAX_ACC)
    {
      command = real_speed - MAX_ACC;
    }

  /* limit speed command (speed setpoint) */
  if (command > final_speed)
    {
      command = final_speed;
    }
  if (command < -final_speed)
    {
      command = -final_speed;
    }
  return command;
}

/**
 *
 */
polar_t
speed_controller (polar_t speed_setpoint, polar_t current_speed)
{
  polar_t speed_error;
  polar_t command;

  speed_error.distance = speed_setpoint.distance - current_speed.distance;
  speed_error.angle = speed_setpoint.angle - current_speed.angle;

  command.distance = pid_controller (&linear_speed_pid, speed_error.distance);
  command.angle = pid_controller (&angular_speed_pid, speed_error.angle);

  return command;
}

polar_t
controller_update (pose_t pose_setpoint, pose_t current_pose, polar_t speed_setpoint,
		   polar_t current_speed)
{
  /********************************* position pid controller ***************************/

  /* compute position error */
  polar_t position_error = compute_error (pose_setpoint, current_pose);

  /* position correction */
  if (position_error.distance > 500)
    {
      position_error.angle -= current_pose.O; // [pulse]

      if (fabs (position_error.angle) > (M_PI * WHEELS_DISTANCE / 2.0))
	{
	  position_error.distance = -position_error.distance;
	  if (position_error.angle < 0)
	    {
	      position_error.angle += (M_PI * WHEELS_DISTANCE);
	    }
	  else
	    {
	      position_error.angle -= (M_PI * WHEELS_DISTANCE);
	    }
	}
    }
  /* orientation correction (position is reached) */
  else
    {
      position_error.distance = 0;
      pid_reset (&linear_pose_pid);

      position_error.angle = pose_setpoint.O - current_pose.O; // [pulse]

      /* orientation is reached */
      if (fabs (position_error.angle) < 100)
	{
	  position_error.angle = 0;
	  pid_reset (&angular_pose_pid);
	  pose_reached = 1;
	}
    }

  /* compute speed command with position pid controller */
  polar_t command;
  command.distance = pid_controller (&linear_pose_pid, position_error.distance);
  command.angle = pid_controller (&angular_pose_pid, position_error.angle);

  /* limit speed command */
  polar_t speed;
  speed.distance = limit_speed_command (command.distance,
					speed_setpoint.distance,
					current_speed.distance);
  speed.angle = limit_speed_command (command.angle, speed_setpoint.angle,
				     current_speed.angle);

  /********************************* speed pid controller ***************************/
  return speed_controller (speed, current_speed);
}
