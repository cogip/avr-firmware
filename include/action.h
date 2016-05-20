/*
 * action.h
 *
 *  Created on: 4 mai 2015
 *      Author: ldo
 */

#ifndef ACTION_H_
#define ACTION_H_

#include <xmega/timer.h>
#include "sd21.h"
#include "sensor.h"
#include "utils.h"
#include "analog_sensor.h"

void
close_clap_arm (void);

void
open_clap_arm (void);

void
action_setup (void);

void
open_right_cup (void);

void
close_right_cup (void);

void
open_left_cup (void);

void
close_left_cup (void);

void
close_pince (void);

void
monter_tour (void);

uint8_t
descendre_tour (void);

void
gestion_tour (void);

void
set_release_right_cup (void);

void
reset_release_right_cup (void);

void
set_release_left_cup (void);

void
reset_release_left_cup (void);

void
attraper_cup (void);

void
open_pince (void);

void
close_door (void);

void
open_door (void);

void
open_half_door (void);

void
spot_up (void);

void
spot_down (void);

#endif /* ACTION_H_ */
