/*
 * encoder.h
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "odometry.h"

void encoder_setup(void);
polar_t encoder_read(void);

#endif /* ENCODER_H_ */
