#include <math.h>
#include <stdio.h>

#include "console.h"
#include "encoder.h"
#include "kos.h"
#include "log.h"
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

static uint16_t tempo;

static void show_game_time()
{
	static uint8_t _secs = (4500 / 90);

	if (! --_secs) {
		_secs = (4500 / 90);
		print_info ("Game time = %d\n",
			    tempo / (4500 / 90));
	}
}

static void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_RIGHT, right_command);
	hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_LEFT,  left_command);

	log_vect_setvalue(&datalog, LOG_IDX_MOTOR_L, (void *) &left_command);
	log_vect_setvalue(&datalog, LOG_IDX_MOTOR_R, (void *) &right_command);
}

void task_controller_update()
{
	polar_t	robot_speed;
	/* bot position on the 'table' (absolute position): */
	pose_t	robot_pose		= POSE_INITIAL;
	pose_t	pose_order		= { 0, 0, 0 };
	polar_t	speed_order		= { 0, 0 };
	polar_t	motor_command;
	func_cb_t pfn_evtloop_end_of_game = mach_get_end_of_game_pfn();
	uint8_t stop = 0;

	for (;;) {
		kos_set_next_schedule_delay_ms(20);

		switch(controller.mode) {
		default:
		case CTRL_STATE_STOP:
		{
			if (pfn_evtloop_end_of_game && tempo >= 4500)
				(*pfn_evtloop_end_of_game)();

			/* final position */
			motor_command.distance = 0;
			motor_command.angle = 0;
			motor_drive(motor_command);

			kos_yield();
		}
		break;

		case CTRL_STATE_IDLE:
		{
			/* No motor control at all (PMW unit tests). */

			kos_yield();
		}

		case CTRL_STATE_INGAME:
		{
			if (tempo >= 4500) {
				controller.mode = CTRL_STATE_STOP;
				break;
			}

			tempo++;
			show_game_time();

			/* catch speed */
			robot_speed = encoder_read();

			/* convert to position */
			odometry_update(&robot_pose, &robot_speed, SEGMENT);

			/* get next pose_t to reach */
			pose_order = mach_trajectory_get_route_update();

			pose_order.x *= PULSE_PER_MM;
			pose_order.y *= PULSE_PER_MM;
			pose_order.O *= PULSE_PER_DEGREE;

			/* collision detection */
			stop = mach_stop_robot();

			if (stop) {
				speed_order.distance = 0;
				speed_order.angle = 0;
			} else {
				/* speed order in position = 60 pulses / 20ms */
				speed_order.distance = 60;
				/* speed order in angle? = 60 pulses / 20ms */
				speed_order.angle = 60;
			}

			/* PID / feedback control */
			motor_command = controller_update(&controller,
							  pose_order,
							  robot_pose,
							  speed_order,
							  robot_speed);

			/* set speed to wheels */
			motor_drive(motor_command);
		}
		break;

#if defined(CONFIG_CALIBRATION)
		case CTRL_STATE_CALIB_MODE1:
		{
			/*
			 * First calibration test:
			 * Perform two PWM sweeps to characterize encoders.
			 */

			/* first entry, we reset the datalog */
			if (!tempo)
				log_vect_reset(&datalog, "cal1_up",
						LOG_IDX_SPEED_L,
						LOG_IDX_SPEED_R,
						LOG_IDX_MOTOR_L,
						LOG_IDX_MOTOR_R,
						-1);
			/*
			 * Two ramps :
			 * 1. [-pwm ... +pwm] for 400 cycles (0.02 = 8s)
			 * 2. [+pwm ... -pwm] for 400 cycles
			 */
			motor_command.angle = 0;
			if (tempo < 50)
				motor_command.distance = -200;
			else if (tempo >= 50 && tempo < 400 - 50)
				motor_command.distance = (int16_t)((double)(tempo - 50) * 4./3.) - 200;
			else if (tempo >= 400 - 50 && tempo < 400 + 50)
				motor_command.distance = 200;
			else if (tempo >= 450 && tempo < 800 - 50)
				motor_command.distance = -((int16_t)((double)(tempo - 450) * 4./3.) - 200);
			else if (tempo >= 800 - 50)
				motor_command.distance = -200;

			motor_drive(motor_command);

			/* catch speed */
			robot_speed = encoder_read();

			log_vect_display_line(&datalog);

			tempo ++;
			if (tempo == 400) {
				//motor_command.distance = 0;
				//motor_command.angle = 0;
				//motor_drive(motor_command);

				log_vect_display_last_line(&datalog);
				/* prepare next log */
				log_vect_reset(&datalog, "cal1_down",
						LOG_IDX_SPEED_L,
						LOG_IDX_SPEED_R,
						LOG_IDX_MOTOR_L,
						LOG_IDX_MOTOR_R,
						-1);
			} else if (tempo == 800) {
				motor_command.distance = 0;
				motor_command.angle = 0;
				motor_drive(motor_command);

				log_vect_display_last_line(&datalog);

				controller.mode = CTRL_STATE_STOP;
				tempo = 0;
			}
		}
		break;

		case CTRL_STATE_CALIB_MODE2:
		{
			/*
			 * Second calibration test:
			 * Perform a speed command to tune Kp, Ki (& Kd).
			 */

			/* first entry, we reset the datalog */
			if (!tempo)
				log_vect_reset(&datalog, "cal2",
						LOG_IDX_ROBOT_SPEED_D,
						/*LOG_IDX_ROBOT_SPEED_A,*/
						LOG_IDX_SPEED_ORDER_D,
						/*LOG_IDX_SPEED_ORDER_A,*/
						LOG_IDX_MOTOR_L,
						LOG_IDX_MOTOR_R,
						-1);
			/*
			 * t[0s..1s] : speed is set to 0
			 * t[1s..7s] : speed is set to full
			 * t[7s..8s] : speed is set to 0
			 */
			if (tempo < 50)
				speed_order.distance = 0;
			else if (tempo >= 50 && tempo < 400 - 50)
				speed_order.distance = 15;
			else if (tempo >= 400 - 50)
				speed_order.distance = 0;

			speed_order.angle = 0;

			log_vect_setvalue(&datalog, LOG_IDX_SPEED_ORDER_D, (void *) &speed_order.distance);

			/* catch speed */
			robot_speed = encoder_read();
			motor_command = speed_controller(&controller,
							 speed_order,
							 robot_speed);

			log_vect_setvalue(&datalog, LOG_IDX_ROBOT_SPEED_D, (void *) &robot_speed.distance);

			motor_drive(motor_command);

			log_vect_display_line(&datalog);

			tempo ++;
			if (tempo == 400) {
				motor_command.distance = 0;
				motor_command.angle = 0;
				motor_drive(motor_command);

				log_vect_display_last_line(&datalog);

				controller.mode = CTRL_STATE_STOP;
				tempo = 0;
			}
		}
		break;
#endif /* defined(CONFIG_CALIBRATION) */

		} /* switch(controller.mode) */

		/* this task is called every scheduler tick (20ms) */
		kos_yield();
	}
}

#if defined(CONFIG_CALIBRATION)
static void controller_calibration_usage(void)
{
	printf("\n>>> Entering controller calibration mode\n\n");

	printf("\t'1' to launch motor sweep [-pwm..+pwm] (out: cal1*.csv)\n");
	printf("\t       this will calibrate : encoders & pwm ranges\n");
	printf("\t'2' to launch speed control loop only (out: cal2.csv)\n");
	printf("\t       this will calibrate : Kp, Ki & (Kd) for speed PID\n");
	printf("\n");
	printf("\t'h' to display this help\n");
	printf("\t'q' to quit\n");
	printf("\n");
}

void controller_enter_calibration()
{
	int c;
	uint8_t quit = 0;

	controller_calibration_usage();

	while (!quit) {

		/* display prompt */
		printf("$ ");

		/* wait for command */
		c = con_getchar();
		printf("%c\n", c);

		switch (c) {
		case '1':
			controller.mode = CTRL_STATE_CALIB_MODE1;
			printf("CAL1 launched\n");
			break;
		case '2':
			controller.mode = CTRL_STATE_CALIB_MODE2;
			printf("CAL2 launched\n");
			break;
		case 'h':
			controller_calibration_usage();
			break;
		case 'q':
			quit = 1;
			break;
		default:
			printf("\n");
			break;
		}
	}
}
#endif /* CONFIG_CALIBRATION */
