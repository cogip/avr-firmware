#include "controller.h"
#include "hbridge.h"
#include "log.h"
#include "platform.h"
#include "platform_task.h"
#include "qdec.h"

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

static polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = qdec_read(&encoders[0]);
	int16_t right_speed = qdec_read(&encoders[1]);

	//print_dbg("encoders(L,R) = (%d, %d)\n", left_speed, right_speed);

	/* update speed */
	robot_speed.distance = (right_speed + left_speed) / 2.0;
	robot_speed.angle = right_speed - left_speed;

	//print_dbg("(dist, angle) = (%d, %d)\n",
	//	  robot_speed.distance, robot_speed.angle);

	return robot_speed;
}

static void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_RIGHT, right_command);
	hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_LEFT,  left_command);
}

uint8_t task_controller_update()
{
	polar_t	robot_speed;
	/* bot position on the 'table' (absolute position): */
	pose_t	robot_pose		= { 1856.75, 0, 0 };
	pose_t	pose_order		= { 0, 0, 0 };
	polar_t	speed_order		= { 60, 60 };
	polar_t	motor_command;
	uint8_t stop = 0;
	
	if (tempo >= 4500) {
		/* final position */
		motor_command.distance = 0;
		motor_command.angle = 0;
		motor_drive(motor_command);

		return TRUE;
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
#if 0
	motor_command = speed_controller(speed_order,
					 robot_speed);
#endif
	motor_command = controller_update(pose_order,
					  robot_pose,
					  speed_order,
					  robot_speed);

	/* set speed to wheels */
	motor_drive(motor_command);

	/* this task is called every scheduler tick (20ms) */
	return FALSE;
}

/*
 * Tasks registration
 */
uint8_t tasks_nb = 1;
task_t tasks_list[] = {
	[0] = {
		.state = TASK_SLEEP,
		.entry_point = task_controller_update,
	},
};

