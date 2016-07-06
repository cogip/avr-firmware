#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "analog_sensor.h"
#include "hbridge.h"
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

#define MAX_ACC				4

#define HBRIDGE_MOTOR_LEFT		0
#define HBRIDGE_MOTOR_RIGHT		1
#define HBRIDGE_MOTOR_TOWER		2

extern analog_sensors_t ana_sensors;
extern qdec_t encoders[];
extern sd21_t sd21;
extern hbridge_t hbridges;

uint8_t mach_detect_start(void);
void mach_evtloop_before_game(void);
void mach_evtloop_in_game(void);
void mach_evtloop_end_of_game(void);

pose_t mach_trajectory_get_route_update(void);
uint8_t mach_stop_robot(void);

#if defined(CONFIG_CALIBRATION)
void mach_check_calibration_mode(void);
#endif

void mach_timer_setup(func_cb_t handler);
void mach_setup(void);

#endif /* PLATFORM_H_ */
