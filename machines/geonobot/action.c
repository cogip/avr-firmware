/*
 * action.c
 *
 *  Created on: 4 mai 2015
 *      Author: ldo
 */
#include "action.h"
#include "analog_sensor.h"
#include "platform.h"
#include "sd21.h"
#include "sensor.h"

/**
 * servo map
 * 1 : Rear right bottom arm
 * 2 : Rear left bottom arm
 * 3 : Right hatch
 * 4 : Left hatch
 * 5 : Right arm
 * 6 : Left arm
 * 7 : Glasses right arm
 * 8 : Glasses left arm
 * 9 : Clamp
 * 10 : Right door
 * 11 : Left door
 *
 * MCC1 : Lift
 */

static uint8_t flag_opened_clamp;
static uint8_t released_right_cup;
static uint8_t released_left_cup;

static uint8_t flag_tower_down = 0;

void set_flag_tower_down(void)
{
	flag_tower_down = 1;
}

void reset_flag_tower_down(void)
{
	flag_tower_down = 0;
}

/**
 * close all
 */
void action_setup(void)
{
	/* TODO: remove static variable already set to 0 (tbc) */
	flag_opened_clamp = 0;
	released_right_cup = 0;
	released_left_cup = 0;
}

void open_clap_arm(void)
{
	sd21_control_servo(&sd21, 4, SD21_SERVO_OPEN);
	sd21_control_servo(&sd21, 5, SD21_SERVO_OPEN);
}

void close_clap_arm(void)
{
	sd21_control_servo(&sd21, 4, SD21_SERVO_CLOSE);
	sd21_control_servo(&sd21, 5, SD21_SERVO_CLOSE);
}

/**
 * OK
 */
void close_right_cup(void)
{
	sd21_control_servo(&sd21, 6, SD21_SERVO_CLOSE);
}

void open_right_cup(void)
{
	sd21_control_servo(&sd21, 6, SD21_SERVO_OPEN);
}

/**
 * open left cup OK
 */
void close_left_cup(void)
{
	sd21_control_servo(&sd21, 7, SD21_SERVO_CLOSE);
}

/**
 * close left cup OK
 */
void open_left_cup(void)
{
	sd21_control_servo(&sd21, 7, SD21_SERVO_OPEN);
}

void close_pince(void)
{
	sd21_control_servo(&sd21, 8, SD21_SERVO_CLOSE);
	flag_opened_clamp = 0;
}

void open_pince(void)
{
	sd21_control_servo(&sd21, 8, SD21_SERVO_OPEN);
	flag_opened_clamp = 1;
}

/**
 * a verifier avec la pile apres montage
 */
void close_door(void)
{
	sd21_control_servo(&sd21, 9, SD21_SERVO_CLOSE);
	sd21_control_servo(&sd21, 10, SD21_SERVO_CLOSE);
}

/**
 * OK
 */
void open_door(void)
{
	sd21_control_servo(&sd21, 9, SD21_SERVO_OPEN);
	sd21_control_servo(&sd21, 10, SD21_SERVO_OPEN);
}

/**
 * OK
 */
void open_half_door(void)
{
#if 0
	/* TODO */
	sd21_control_servo(&sd21, 9, SD21_SERVO_OPEN_HALF);
	sd21_control_servo(&sd21, 10, SD21_SERVO_OPEN_HALF);

	sd21_control_servo(9, 0, 1250);
	sd21_control_servo(10, 0, 900);
#endif
}

void spot_up(void)
{
	/* elevator */
	if (detect_elevator_up())
		hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_TOWER, 0);
	else
		hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_TOWER, -200);
}

/**
 *
 */
void spot_down(void)
{
	/* elevator */
	if (detect_elevator_down())
		hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_TOWER, 0);
	else
		hbridge_engine_update(&hbridges, HBRIDGE_MOTOR_TOWER, 200);
}

static uint8_t spot_get_up = 1;
static uint8_t spot_get_down;

void monter_tour(void)
{
	/* elevator is up or get down */
	if ((detect_spot() && detect_elevator_up()) || (spot_get_down == 1)) {
		open_pince();
		spot_down();
		spot_get_down = 1;
		spot_get_up = 0;

		if (detect_elevator_up())
			close_door();
	}

	/* elevation is down or get up */
	if (detect_elevator_down() || (spot_get_up == 1)) {
		close_pince();
		spot_up();
		spot_get_up = 1;
		spot_get_down = 0;
	}
}

void descendre_tour(void)
{
	spot_down();

	if (detect_elevator_down()) {
		open_door();
		open_pince();
	} else
		open_half_door();
}

void gestion_tour(void)
{
	if (flag_tower_down == 0)
		monter_tour();
	else
		descendre_tour();
}

void set_release_right_cup(void)
{
	released_right_cup = 1;
}

void reset_release_right_cup(void)
{
	released_right_cup = 0;
}

void set_release_left_cup(void)
{
	released_left_cup = 1;
}

void reset_release_left_cup(void)
{
	released_left_cup = 0;
}

void attraper_cup(void)
{
	if (detect_right_cup() && !released_right_cup)
		close_right_cup();
	else
		open_right_cup();

	if (detect_left_cup() && !released_left_cup)
		close_left_cup();
	else
		open_left_cup();
}

/* TODO: a 'speed' entry in route_t should be a more generic way to do this */
uint8_t action_require_stop_robot()
{
	uint8_t stop = 0;

	if (((detect_spot()) && (!detect_elevator_down()))
	    || (!detect_elevator_up() && !flag_tower_down)
	    || (!detect_elevator_down() && flag_tower_down)
	   )
		stop = 1;

	return stop;
}
