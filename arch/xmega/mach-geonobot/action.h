/*
 * action.h
 *
 *  Created on: 4 mai 2015
 *      Author: ldo
 */

#ifndef ACTION_H_
#define ACTION_H_

#include <stdint.h>

#include "odometry.h"
#include "utils.h"

typedef struct {
	pose_t p;
	func_cb_t action_function;
	uint8_t can_retro;
} action_t;

void action_setup(void);
void gestion_tour(void);
void set_flag_tower_down(void);
void reset_flag_tower_down(void);

void open_clap_arm(void);
void close_clap_arm(void);
void open_right_cup(void);
void close_right_cup(void);
void open_left_cup(void);
void close_left_cup(void);
void close_pince(void);
void monter_tour(void);
void descendre_tour(void);
void set_release_right_cup(void);
void reset_release_right_cup(void);
void set_release_left_cup(void);
void reset_release_left_cup(void);
void attraper_cup(void);
void open_pince(void);
void close_door(void);
void open_door(void);
void open_half_door(void);
void spot_up(void);
void spot_down(void);

uint8_t action_require_stop_robot();

#endif /* ACTION_H_ */
