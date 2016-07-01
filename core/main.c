/**
 * \file main.c
 *
 * \date 11 mars 2015
 * \author ldo
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include <xmega/clksys.h>
#include <xmega/timer.h>
#include <xmega/twi.h>
#include <xmega/usart.h>

#include "action.h"
#include "analog_sensor.h"
#include "controller.h"
#include "encoder.h"
#include "sd21.h"
#include "sensor.h"
#include "odometry.h"
#include "platform.h"
#include "route.h"

static uint8_t next_timeslot_trigged;
uint8_t	pose_reached;
static int16_t	tempo;

static void interrupt_setup(void)
{
	/* Programmable Multilevel Interrupt Controller */
	PMIC.CTRL |= PMIC_LOLVLEN_bm; /* Low-level Interrupt Enable */
}

/* Timer 0 Overflow interrupt */
static void irq_timer_tcc0_handler(void)
{
	next_timeslot_trigged = 1;
}

/**
 *
 */
static void setup(void)
{
#if F_CPU == 32000000UL
	clksys_intrc_32MHz_setup();
#endif
	mach_pinmux_setup();

	/* setup analog conversion */
	adc_setup(&ADCA, irq_adc_handler);

	interrupt_setup();

	/* global interrupt enable */
	sei();

	/* timer setup */
	next_timeslot_trigged = 0;
	timer_0_register_ovf_cb(irq_timer_tcc0_handler);

	/* TCC0 ClkIn == ClkPer / 1024 == 31.25 KHz */
	/* Counter set to 625 for 50Hz output (20ms) */
	timer_normal_mode_setup(&TCC0, 625, TC_CLKSEL_DIV1024_gc);

	/* setup usart communication */
	xmega_usart_setup(&USARTC0);

	/* setup TWI communication with SD21 */
	sd21_setup(&TWIC);

	action_setup(); /* TODO: commenter pour debug */

	/* TODO: following should be in platform */
	hbridge_setup(&hbridges);

	/* setup qdec */
	encoder_setup();

	/* controller setup */
	odometry_setup(WHEELS_DISTANCE);
	controller_setup();
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

	setup();

	/* start first conversion */
	adc_read(&ADCA, 0);
	xmega_usart_transmit(&USARTC0, 0xAA);

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
		read_analog_sensor();
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
