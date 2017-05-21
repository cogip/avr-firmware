#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "analog_sensor.h"
#include "controller.h"
#include "hbridge.h"
#include "log.h"
#include "odometry.h"
#include "qdec.h"
#include "sd21.h"

/*
 * Machine parameters
 */

/* To be computed :
 *  - PULSE_PER_MM		: Number of pulses per mm of coding wheel
 *  - WHEELS_DISTANCE		: Distance between coding wheels in pulses
 *  - PULSE_PER_DEGREE		: Number of pulses per degree of coding wheel
 *
 * Must be known :
 *  - WHEELS_DIAMETER		: Coding wheel diameter
 *  - WHEELS_DISTANCE_MM	: Distance between coding wheels in mm
 *
 * Must be known and defined :
 *  - WHEELS_ENCODER_RESOLUTION	: Number of pulses by turn of coding wheels
 */

#define WHEELS_ENCODER_RESOLUTION	2000
/* WHEELS_PERIMETER = pi*WHEELS_DIAMETER
 * PULSE_PER_MM = WHEELS_ENCODER_RESOLUTION / WHEELS_PERIMETER
 */
#define PULSE_PER_MM			1.0
/* WHEELS_DISTANCE = WHEELS_DISTANCE_MM * PULSE_PER_MM */
#define WHEELS_DISTANCE			2960
/* WHEELS_DISTANCE*pi pulses pour 360° soit 25.88 pulses/° */
#define PULSE_PER_DEGREE		1.0

#define POSE_INITIAL			{ 0, 0, 0 }
#define MAX_ACC				200

#define HBRIDGE_MOTOR_LEFT		0
#define HBRIDGE_MOTOR_RIGHT		1

#define USART_CONSOLE			USARTC0

extern analog_sensors_t ana_sensors;
extern qdec_t encoders[];
extern sd21_t sd21;
extern hbridge_t hbridges;

extern controller_t controller;

extern datalog_t datalog;

func_cb_t mach_get_ctrl_loop_pre_pfn();
func_cb_t mach_get_ctrl_loop_post_pfn();
func_cb_t mach_get_end_of_game_pfn();
pose_t mach_trajectory_get_route_update(void);
uint8_t mach_is_zone_obscured(analog_sensor_zone_t zone);
uint8_t mach_is_game_launched(void);
uint8_t mach_is_camp_yellow(void);

#if defined(CONFIG_CALIBRATION)
void mach_check_calibration_mode(void);
#endif

void mach_setup(void);

void mach_tasks_init();
void mach_sched_init();
void mach_sched_run();

#endif /* PLATFORM_H_ */
