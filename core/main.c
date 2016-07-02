/**
 * \file main.c
 *
 * \date 11 mars 2015
 * \author ldo
 */

#include "action.h"
#include "analog_sensor.h"
#include "controller.h"
#include "sensor.h"
#include "log.h"
#include "odometry.h"
#include "platform.h"
#include "route.h"

uint8_t next_timeslot_trigged;
uint8_t	pose_reached;
static int16_t	tempo;

/* Timer 0 Overflow interrupt */
static void irq_timer_tcc0_handler(void)
{
	next_timeslot_trigged = 1;
}

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = qdec_read(&encoders[0]);
	int16_t right_speed = qdec_read(&encoders[1]);

	print_dbg("encoders(L,R) = (%d, %d)\n", left_speed, right_speed);

	/* update speed */
	robot_speed.distance = (right_speed + left_speed) / 2.0;
	robot_speed.angle = right_speed - left_speed;

	print_dbg("(dist, angle) = (%d, %d)\n",
		  robot_speed.distance, robot_speed.angle);

	return robot_speed;
}

void motor_drive(polar_t command)
{
	/************************ commandes moteur ************************/
	int16_t right_command = (int16_t) (command.distance + command.angle);
	int16_t left_command = (int16_t) (command.distance - command.angle);

	hbridge_engine_update(&hbridges, &hbridges.engines[HBRIDGE_MOTOR_RIGHT], right_command);
	hbridge_engine_update(&hbridges, &hbridges.engines[HBRIDGE_MOTOR_LEFT],  left_command);
}

/**
 * TODO 90 s
 * TODO gestion bras maintien position plus fermé
 * TODO init trappe pop corn
 */
int main(void)
{
	polar_t	robot_speed;
	polar_t	speed_setpoint		= { 60, 60 };
	polar_t	motor_command;
	pose_t	robot_pose		= { 1856.75, 0, 0 }; /* position absolue */
	pose_t	pose_setpoint		= { 0, 0, 0 };
	uint8_t all_irs[] = { 2, 3, 4, 5, 6, 7 };
	uint8_t stop = 0;

	mach_setup();
	mach_timer_setup(irq_timer_tcc0_handler);

#if 0
	/* start first conversion */
	adc_read(&ADCA, 0);
#endif

	/* controller setup */
	odometry_setup(WHEELS_DISTANCE);
	controller_setup();

	while (detect_start())
		gestion_tour();

	/* main loop == 90s */
	while (tempo < 4500) {
		if (next_timeslot_trigged) {
			/* we enter here every 20ms */
			tempo++;

			/* catch speed */
			robot_speed = encoder_read();

			/* convert to position */
			odometry_update(&robot_pose, &robot_speed, SEGMENT);

			/* get next pose_t to reach */
			pose_setpoint = route_update();

			pose_setpoint.x *= PULSE_PER_MM;
			pose_setpoint.y *= PULSE_PER_MM;
			pose_setpoint.O *= PULSE_PER_DEGREE;

			/* mirror mode: invert path regarding bot's camp */
			if (detect_color()) {
				pose_setpoint.y *= -1;
				pose_setpoint.O *= -1;
			}

			/* collision detect */

#if 0
			uint8_t side_irs[] = { 0, 1 };
			uint8_t rear_irs[] = { 2, 3 };
			uint8_t front_irs[] = { 4, 5, 6, 7 };

			if ((right_command > 0) && (left_command > 0))
			stop = stop_robot(front_irs, 4);
			else if ((right_command < 0) && (left_command < 0))
			stop = stop_robot(rear_irs, 2);
			else
#endif
			stop = stop_robot(all_irs, 6);

			if (stop && get_route_index()) {
				speed_setpoint.distance = 0;
				speed_setpoint.angle = 0;
#if 0
				if ((stop > 1) && (get_can_retro()))
				down_route_index();
#endif
			} else {
				speed_setpoint.distance = 60;
				speed_setpoint.angle = 60;
			}

			/* PID / feedback control */
#if 0
			motor_command = speed_controller(speed_setpoint,
							 robot_speed);
#endif
			motor_command = controller_update(pose_setpoint,
							  robot_pose,
							  speed_setpoint,
							  robot_speed);

			/* set speed to wheels */
			motor_drive(motor_command);

			next_timeslot_trigged = 0;
		}

		attraper_cup();
		analog_sensor_read();
		gestion_tour();
	}

	/* final position */
	while (1) {
		open_pince();
		open_door();
		set_release_right_cup();
		set_release_left_cup();
		motor_command.distance = 0;
		motor_command.angle = 0;
		motor_drive(motor_command);
	}

	/* we never reach this point */
	return 0;
}
