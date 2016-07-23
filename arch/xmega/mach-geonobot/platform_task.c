#include <stdio.h>
#include <stdlib.h>

#include "action.h"
#include "controller.h"
#include "hbridge.h"
#include "kos.h"
#include "log.h"
#include "platform.h"
#include "platform_task.h"
#include "qdec.h"
#include "sensor.h"

static uint16_t tempo;

static void mach_evtloop_end_of_game(void)
{
	open_pince();
	open_door();
	set_release_right_cup();
	set_release_left_cup();
}


void task_active_event_loop()
{
	printf("task_active_event_loop()\n");
	for (;;) {
		if (detect_start()) {
			gestion_tour();
		} else {
			attraper_cup();
			gestion_tour();
		}
	}
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

void task_controller_update()
{
	polar_t	robot_speed;
	/* bot position on the 'table' (absolute position): */
	pose_t	robot_pose		= { 1856.75, 0, 0 };
	pose_t	pose_order		= { 0, 0, 0 };
	polar_t	speed_order		= { 60, 60 };
	polar_t	motor_command;
	uint8_t stop = 0;

	for (;;) {
		if (tempo >= 4500) {
			/* final position */
			mach_evtloop_end_of_game();

			motor_command.distance = 0;
			motor_command.angle = 0;
			motor_drive(motor_command);

			continue;
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
		//kos_schedule();
	}
}

#if defined(CONFIG_CALIBRATION)
static void mach_calibration_usage(void)
{
	printf("\n>>> Entering calibration mode\n\n");

	printf("\t's' to calibrate servos (sd21 card)\n");
	printf("\n");
	printf("\t'h' to display this help\n");
	printf("\t'e' to exit calibration mode\n");
	printf("\n");
}

static void mach_enter_calibration_mode(void)
{
	int c;
	uint8_t quit = 0;

	mach_calibration_usage();
	/* NOTE: instead of a gpio status, use the flag to activate the
	 * feature all the time. Also, add a autoboot feature, ask the user to
	 * press a key & if no key start normally after 3 seconds
	 */

	while (!quit) {

		/* display prompt */
		printf("$ ");

		/* wait for command */
		c = getchar();
		printf("%c\n", c);

		switch (c) {
		case 's':
			sd21_enter_calibration(&sd21);
			break;
		case 'h':
			mach_calibration_usage();
			break;
		case 'e':
			quit = 1;
			break;
		default:
			printf("\n");
			break;
		}
	}

	for(;;) ;
}
#endif /* CONFIG_CALIBRATION */

void task_idle()
{
	for(;;);
}

/*
 * Tasks registration
 */

#define TASK_IDLE_STACK		96
#define TASK_CALIB_STACK	184
#define TASK_EVENT_STACK	128
#define TASK_CTRL_STACK		256

void mach_tasks_init()
{
	kos_init();

	uint8_t *stack_idle = malloc(TASK_IDLE_STACK);
#if defined(CONFIG_CALIBRATION)
	uint8_t *stack_calibration = malloc(TASK_CALIB_STACK);
#endif
	uint8_t *stack_active_event_loop = malloc(TASK_EVENT_STACK);
	uint8_t *stack_controller_update = malloc(TASK_CTRL_STACK);

	kos_new_task(task_idle, "IDLE", stack_idle, TASK_IDLE_STACK);
#if defined(CONFIG_CALIBRATION)
	kos_new_task(mach_enter_calibration_mode, "CALIB", stack_calibration, TASK_CALIB_STACK);
#endif
	kos_new_task(task_active_event_loop, "EVNT", stack_active_event_loop, TASK_EVENT_STACK);
	kos_new_task(task_controller_update, "CTRL", stack_controller_update, TASK_CTRL_STACK);
}
