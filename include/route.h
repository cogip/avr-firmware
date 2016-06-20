/*
 * route.h
 *
 *  Created on: 2 mai 2015
 *      Author: ldo
 */

#ifndef ROUTE_H_
#define ROUTE_H_

#include <stdint.h>

#include "odometry.h"

pose_t route_update(void);
void down_route_index(void);
uint8_t get_can_retro(void);
uint8_t get_route_index(void);

#endif /* ROUTE_H_ */
