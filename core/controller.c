#include <math.h>
#include <stdio.h>

#include "console.h"
#include "encoder.h"
#include "kos.h"
#include "log.h"
#include "odometry.h"
#include "platform.h"
#include "trigonometry.h"

#include "controller.h"

static uint16_t tempo;

/**
 * \fn polar_t compute_error(const pose_t p1, const pose_t p2)
 * \brief compute error between 2 poses
 * \param p1 : setpoint pose
 * \param p2 : measure pose
 * \return distance and angle errors between 2 poses
 */
static polar_t compute_error(controller_t *ctrl,
			     const pose_t pose_order, const pose_t pose_current)
{
	polar_t error;
	double x, y, O;

	x = pose_order.x - pose_current.x;
	y = pose_order.y - pose_current.y;

	O = limit_angle_rad(atan2(y, x) - DEG2RAD(pose_current.O));

	error.angle = RAD2DEG(O);
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
	double a = command - real_speed;

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
	polar_t command;
	polar_t speed;
	/* ******************** position pid controller ******************** */

	/* compute position error */
	polar_t position_error;

	position_error = compute_error(ctrl, pose_order, pose_current);

	ctrl->pose_reached = 0;

	cons_printf("%+.0f,%+.0f,%+.0f,%+.0f,%+.0f,%+.0f,"
		    "%+.0f,%+.0f,"
		    "%+.0f,%+.0f,"
		    "%+.0f,%+.0f,"
		    "%d,"
		    "\n",
			  pose_order.x / PULSE_PER_MM,
			  pose_order.y / PULSE_PER_MM,
			  pose_order.O,
			  pose_current.x / PULSE_PER_MM,
			  pose_current.y / PULSE_PER_MM,
			  pose_current.O,
			  position_error.distance / PULSE_PER_MM,
			  position_error.angle,
			  speed_order.distance / PULSE_PER_MM,
			  speed_order.angle,
			  speed_current.distance / PULSE_PER_MM,
			  speed_current.angle,
			  tempo);

	/* position correction */
	if (ctrl->regul != CTRL_REGUL_POSE_ANGL
	    && fabs(position_error.distance) > ctrl->min_distance_for_angular_switch) {

#if 0
		/* should we go reverse? */
		if (fabs(position_error.angle) > 90) {
			position_error.distance = -position_error.distance;

			if (position_error.angle < 0)
				position_error.angle += 180;
			else
				position_error.angle -= 180;
		}
#endif

		/* if target point direction angle is too important, bot rotates on its starting point */
		if (fabs(position_error.angle) > ctrl->min_angle_for_pose_reached / PULSE_PER_DEGREE) {
			position_error.distance = 0;
			pid_reset(&ctrl->linear_pose_pid);
		}
	} else {
		/* orientation correction (position is reached) */
		ctrl->regul = CTRL_REGUL_POSE_ANGL;

		/* final orientation error */
		if (!ctrl->pose_intermediate)
		{
			position_error.angle = limit_angle_deg(pose_order.O - pose_current.O);
		}

		position_error.distance = 0;
		pid_reset(&ctrl->linear_pose_pid);

		/* orientation is reached */
		if (fabs(position_error.angle) < ctrl->min_angle_for_pose_reached / PULSE_PER_DEGREE) {
			position_error.angle = 0;
			pid_reset(&ctrl->angular_pose_pid);

			ctrl->pose_reached = 1;
			ctrl->regul = CTRL_REGUL_POSE_DIST; //CTRL_REGUL_IDLE;
			print_info ("pose_reached\n");
		}
	}

	position_error.angle *= PULSE_PER_DEGREE;

	/* compute speed command with position pid controller */
	command.distance = pid_controller(&ctrl->linear_pose_pid,
					  position_error.distance);
	command.angle = pid_controller(&ctrl->angular_pose_pid,
				       position_error.angle);


	/* limit speed command */
	speed.distance = limit_speed_command(command.distance,
					     speed_order.distance,
					     speed_current.distance);
	speed.angle = limit_speed_command(command.angle,
					  speed_order.angle,
					  speed_current.angle);

	/* ********************** speed pid controller ********************* */
	return speed_controller(ctrl, speed, speed_current);
}

inline void controller_set_pose_intermediate(controller_t *ctrl, uint8_t intermediate)
{
	ctrl->pose_intermediate = intermediate;
}

inline uint8_t controller_get_pose_reached(controller_t *ctrl)
{
	return ctrl->pose_reached;
}

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
	polar_t	motor_command		= { 0, 0 };
	func_cb_t pfn_evtloop_prefunc  = mach_get_ctrl_loop_pre_pfn();
	func_cb_t pfn_evtloop_postfunc = mach_get_ctrl_loop_post_pfn();
	func_cb_t pfn_evtloop_end_of_game = mach_get_end_of_game_pfn();
	uint8_t stop = 0;

	robot_pose.x *= PULSE_PER_MM;
	robot_pose.y *= PULSE_PER_MM;
	robot_pose.O *= PULSE_PER_DEGREE;
	controller.regul = CTRL_REGUL_POSE_DIST;

	print_info ("Start ctrl loop\n");

	for (;;) {
		kos_set_next_schedule_delay_ms(20);

		/* Machine specific stuff, if required */
		if (pfn_evtloop_prefunc)
			(*pfn_evtloop_prefunc)();

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
			robot_speed = encoder_read();

			/* No motor control at all (PMW unit tests). */

			kos_yield();
		}
		break;

		case CTRL_STATE_INGAME:
		{
			/* while starter switch is not release we wait */
			if (!mach_is_game_launched()) {
				kos_yield();
				break;
			}

			if (tempo >= 4500) {
				cons_printf(">>>>\n");
				controller.mode = CTRL_STATE_STOP;
				break;
			}

			if (!tempo) {
				cons_printf("<<<< polar_simu.csv\n");
				cons_printf("pose_order_x,pose_order_y,pose_order_a,"
					    "pose_current_x,pose_current_y,pose_current_a,"
					    "position_error_l,position_error_a,"
					    "speed_order_l,speed_order_a,"
					    "speed_current_l,speed_current_a,"
					    "tempo,"
					    "\n");
			}

			tempo++;
			show_game_time();

			/* catch speed */
			robot_speed = encoder_read();

			/* convert to position */
			odometry_update(&robot_pose, &robot_speed, SEGMENT);

			robot_pose.O /= PULSE_PER_DEGREE;

			/* get next pose_t to reach */
			pose_order = mach_trajectory_get_route_update();

			pose_order.x *= PULSE_PER_MM;
			pose_order.y *= PULSE_PER_MM;

			/* collision detection */
			stop = mach_stop_robot();

			if (stop) {
				speed_order.distance = 0;
				speed_order.angle = 0;
			} else {
				/* speed order in position = 60 pulses / 20ms */
				speed_order.distance = 150;
				/* speed order in angle? = 60 pulses / 20ms */
				speed_order.angle = 150 / 2;
			}

			/* PID / feedback control */
			motor_command = controller_update(&controller,
							  pose_order,
							  robot_pose,
							  speed_order,
							  robot_speed);

			robot_pose.O *= PULSE_PER_DEGREE;
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
			if (!tempo) {
				encoder_reset();
				log_vect_reset(&datalog, "cal2",
						LOG_IDX_ROBOT_SPEED_D,
						/*LOG_IDX_ROBOT_SPEED_A,*/
						LOG_IDX_SPEED_ORDER_D,
						/*LOG_IDX_SPEED_ORDER_A,*/
						LOG_IDX_MOTOR_L,
						LOG_IDX_MOTOR_R,
						-1);
			}
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

		case CTRL_STATE_CALIB_MODE3:
		{
			/*
			 * Third calibration test:
			 * Perform a speed command to tune Kp, Ki (& Kd).
			 */

			/* first entry, we reset the datalog */
			if (!tempo) {
				encoder_reset();
				log_vect_reset(&datalog, "cal3",
						LOG_IDX_ROBOT_SPEED_D,
						/*LOG_IDX_ROBOT_SPEED_A,*/
						LOG_IDX_SPEED_ORDER_D,
						/*LOG_IDX_SPEED_ORDER_A,*/
						LOG_IDX_MOTOR_L,
						LOG_IDX_MOTOR_R,
						-1);
			}
			/*
			 * t[0s..2s] : angular speed is set to +15
			 * t[2s..4s] : angular speed is set to -30
			 * t[4s..6s] : angular speed is set to +30
			 * t[6s..8s] : angular speed is set to -15
			 */
			if (tempo < 100)
				speed_order.angle = +15;
			else if (tempo >= 100 && tempo < 200)
				speed_order.angle = -30;
			else if (tempo >= 200 && tempo < 300)
				speed_order.angle = +30;
			else if (tempo >= 300 && tempo < 400)
				speed_order.angle = -15;

			speed_order.distance = 0;

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

		/* Machine specific stuff, if required */
		if (pfn_evtloop_postfunc)
			(*pfn_evtloop_postfunc)();

		/* this task is called every scheduler tick (20ms) */
		kos_yield();
	}
}

#if defined(CONFIG_CALIBRATION)
static PID_t *controller_get_pid_from_idx(const uint8_t i)
{
	switch(i) {
	case 0:
		return &controller.linear_speed_pid;
	case 1:
		return &controller.angular_speed_pid;
	case 2:
		return &controller.linear_pose_pid;
	case 3:
		return &controller.angular_pose_pid;
	default:
		return NULL;
	}
}

static const char *controller_get_pid_name_from_idx(const uint8_t i)
{
	switch(i) {
	case 0:
		return "linear_speed_pid";
	case 1:
		return "angular_speed_pid";
	case 2:
		return "linear_pose_pid";
	case 3:
		return "angular_pose_pid";
	default:
		return NULL;
	}
}

static void controller_calibration_usage(const uint8_t pid_idx)
{
	cons_printf("\n>>> Entering controller calibration mode\n\n");

	cons_printf("\t'1' to launch motor sweep [-pwm..+pwm] (out: cal1*.csv)\n");
	cons_printf("\t       this will calibrate : encoders & pwm ranges\n");
	cons_printf("\t'2' to launch speed control loop only (out: cal2.csv)\n");
	cons_printf("\t       this will calibrate : Kp, Ki & (Kd) for speed PID\n");
	cons_printf("\t'3' to launch angular control loop only (out: cal3.csv)\n");
	cons_printf("\n");
	cons_printf("\t'v' to dump all PID values\n");
	cons_printf("\t'n' to select next PID to be tuned\n");
	cons_printf("\t'b' to select prev PID to be tuned\n");
	cons_printf("\n");
	cons_printf("\%s:\t"
	       "Kp = %+.2f\tKi = %+.2f\tKd = %+.2f\n",
	       controller_get_pid_name_from_idx(pid_idx),
	       controller_get_pid_from_idx(pid_idx)->kp,
	       controller_get_pid_from_idx(pid_idx)->ki,
	       controller_get_pid_from_idx(pid_idx)->kd);
	cons_printf("\t'p' to tune Kp\n");
	cons_printf("\t'i' to tune Ki\n");
	cons_printf("\t'd' to tune Kd\n");
	cons_printf("\n");
	cons_printf("\t'h' to display this help\n");
	cons_printf("\t'q' to quit\n");
	cons_printf("\n");
}

static void scanf_update_val (const char *var_name, double *var)
{
	cons_printf("%s = %+.2f\tenter new value: ", var_name , *var);
	cons_scanf("%lf", var);
	cons_printf("new %s = %+.2f\n", var_name, *var);
}

void controller_enter_calibration()
{
	int c, i;
	uint8_t quit = 0;
	uint8_t pid_idx = 0;
	PID_t *cur_pid = controller_get_pid_from_idx(pid_idx);

	controller_calibration_usage(pid_idx);

	while (!quit) {

		/* display prompt */
		cons_printf("$ ");

		/* wait for command */
		c = cons_getchar();
		cons_printf("%c\n", c);

		switch (c) {
		case '1':
			controller.mode = CTRL_STATE_CALIB_MODE1;
			cons_printf("CAL1 launched\n");
			break;
		case '2':
			controller.mode = CTRL_STATE_CALIB_MODE2;
			cons_printf("CAL2 launched\n");
			break;
		case '3':
			controller.mode = CTRL_STATE_CALIB_MODE3;
			cons_printf("CAL3 launched\n");
			break;
		case 'p':
			scanf_update_val("Kp", &cur_pid->kp);
			break;
		case 'i':
			scanf_update_val("Ki", &cur_pid->ki);
			break;
		case 'd':
			scanf_update_val("Kd", &cur_pid->kd);
			break;
		case 'n':
			pid_idx += 1;
			pid_idx %= 4;
			cur_pid = controller_get_pid_from_idx(pid_idx);
			controller_calibration_usage(pid_idx);
			break;
		case 'b':
			if (!pid_idx)
				pid_idx = 3;
			else
				pid_idx -= 1;
			cur_pid = controller_get_pid_from_idx(pid_idx);
			controller_calibration_usage(pid_idx);
			break;
		case 'v':
			for (i = 0; i < 4; i++) {
				cons_printf("\%s:\t"
				       "Kp = %+.2f\tKi = %+.2f\tKd = %+.2f\n",
				       controller_get_pid_name_from_idx(i),
				       controller_get_pid_from_idx(i)->kp,
				       controller_get_pid_from_idx(i)->ki,
				       controller_get_pid_from_idx(i)->kd);
			}
			break;
		case 'h':
			controller_calibration_usage(pid_idx);
			break;
		case 'q':
			quit = 1;
			break;
		default:
			cons_printf("\n");
			break;
		}
	}
}
#endif /* CONFIG_CALIBRATION */
