#include <stdio.h>
#include <stdlib.h>

#include "controller.h"
#include "encoder.h"
#include "hbridge.h"
#include "kos.h"
#include "log.h"
#include "mcurses.h"
#include "platform.h"
#include "platform_task.h"
#include "usart.h"
#include "qdec.h"

static uint8_t game_started;
static uint16_t tempo;
polar_t	speed_order		= { 0, 0 };

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
}

void task_controller_update()
{
	polar_t	robot_speed;
	/* bot position on the 'table' (absolute position): */
	pose_t	robot_pose		= { 0, 0, 0 };
	pose_t	pose_order		= { 0, 0, 0 };
	polar_t	motor_command;
	uint8_t stop = 0;

	for (;;) {
		kos_set_next_schedule_delay_ms(20);

		if (tempo >= 4500) {
			/* final position */
			motor_command.distance = 0;
			motor_command.angle = 0;
			motor_drive(motor_command);

			kos_yield();
			continue;
		}

		if (game_started) {
			tempo++;
			show_game_time();
		}

		/* catch speed */
		robot_speed = encoder_read();

		/* convert to position */
		odometry_update(&robot_pose, &robot_speed, SEGMENT);

		/* get next pose_t to reach */
		pose_order = mach_trajectory_get_route_update();

		//pose_order.x *= PULSE_PER_MM;
		//pose_order.y *= PULSE_PER_MM;
		//pose_order.O *= PULSE_PER_DEGREE;

		/* collision detection */
		stop = mach_stop_robot();

		if (stop) {
			speed_order.distance = 0;
			speed_order.angle = 0;
		} else if (game_started) {
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
		motor_command = controller_update(&controller,
						  pose_order,
						  robot_pose,
						  speed_order,
						  robot_speed);

#if defined(CONFIG_CALIBRATION)
		static uint8_t _cpt = 25;

		if (! --_cpt) {
			_cpt = (25);
			mcurses_monitor_printf(0, "robot_speed: [dist:%+.2f, ang:%+.2f]  ",
					robot_speed.distance, robot_speed.angle);
			mcurses_monitor_printf(1, "robot_pose: [x:%+.2f, y:%+.2f, O:%+.2f]  ",
					robot_pose.x, robot_pose.y, robot_pose.O);
			mcurses_monitor_printf(2, "speed_order: [dist:%+.2f, ang:%+.2f]  ",
					speed_order.distance, speed_order.angle);
			mcurses_monitor_printf(3, "pose_order: [x:%+.2f, y:%+.2f, O:%+.2f]  ",
					pose_order.x, pose_order.y, pose_order.O);

			mcurses_monitor_printf(4, "motor: [dist:%+.2f, ang:%+.2f]  ",
					motor_command.distance, motor_command.angle);
		}
#endif /* CONFIG_CALIBRATION */

		/* set speed to wheels */
		motor_drive(motor_command);

		/* this task is called every scheduler tick (20ms) */
		kos_yield();
	}
}

#if defined(CONFIG_CALIBRATION)
static void mach_calibration_usage(void)
{
	printf("\n>>> Entering calibration mode\n\n");

	printf("\t'o' to calibrate odometry\n");
	printf("\t'p' to calibrate hbridge & PWM ctrl\n");
	printf("\t's' to calibrate servos (sd21 card)\n");
	printf("\t'r' to calibrate speed_controller\n");
	printf("\n");
	printf("\t'h' to display this help\n");
	printf("\t'e' to exit calibration mode\n");
	printf("\n");
}

static void mach_enter_calibration_mode(void)
{
	int16_t autoboot_ms = 3000;
	int c;
	uint8_t quit = 0;

	/* wait for keypress, or schedule */
	while (!usart_is_data_arrived(&USART_CONSOLE)) {

		printf("Press a key to enter calibration... %ds remaining\r",
			autoboot_ms / 1000);

		kos_set_next_schedule_delay_ms(250);
		autoboot_ms -= 250;

		if (autoboot_ms > 0)
			kos_yield();
		else
			/* time elapsed, we bypass calibration
			 * and continue to game mode. */
			goto exit_point;
	}
	printf("\n\n");
	getchar();

	mcurses_init();
	mach_calibration_usage();

	while (!quit) {

		/* display prompt */
		printf("$ ");

		/* wait for command */
		c = mach_getchar_or_yield();
		printf("%c\n", c);

		switch (c) {
		case 'o':
			encoder_enter_calibration();
			/* TODO; odometry_enter_calibration */
			break;
		case 'p':
			hbridge_enter_calibration(&hbridges);
			break;
		case 's':
			sd21_enter_calibration(&sd21);
			break;
		case 'r':
			speed_controller_enter_calibration(&speed_order);
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

exit_point:
	game_started = TRUE;
	printf("calibration ended\n");
	kos_task_exit();
}
#endif /* CONFIG_CALIBRATION */

/*
 * Tasks registration
 */

#define TASK_CALIB_STACK	184
#define TASK_CTRL_STACK		256

void mach_tasks_init()
{
	kos_init();

	game_started = FALSE;

#if defined(CONFIG_CALIBRATION)
	kos_new_task(mach_enter_calibration_mode, "CALIB", TASK_CALIB_STACK);
#endif
	kos_new_task(task_controller_update, "CTRL", TASK_CTRL_STACK);
}
