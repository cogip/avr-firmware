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
#define WHEELS_DISTANCE			2960 /* pulses soit 279 mm */
#define WHEELS_DIAMETER			60   /* 60 mm - perimeter : 188 mm */
#define WHEELS_ENCODER_RESOLUTION	2000 /* 500*4 pulse per rotate - 10.61 pulse/mm */

/* 2960*pi/2 = 4650 pulses pour 90° soit 51.66 pulses/° */
#define PULSE_PER_DEGREE		51.66
#define PULSE_PER_MM			10.61

#define POSE_INITIAL			{ 1856.75, 0, 0 }
#define MAX_ACC				4

#define HBRIDGE_MOTOR_LEFT		0
#define HBRIDGE_MOTOR_RIGHT		1
#define HBRIDGE_MOTOR_TOWER		2

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
uint8_t mach_stop_robot(void);

#if defined(CONFIG_CALIBRATION)
void mach_check_calibration_mode(void);
#endif

void mach_setup(void);

void mach_tasks_init();
void mach_sched_init();
void mach_sched_run();

#endif /* PLATFORM_H_ */
