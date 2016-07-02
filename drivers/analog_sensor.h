/*
 * analog_sensor.h
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */

#ifndef ANALOG_SENSOR_H_
#define ANALOG_SENSOR_H_

#include <xmega/adc.h>

#define GP2Y0A41_DIST_MIN	4
#define GP2Y0A41_DIST_MAX	30

#define GP2Y0A21_DIST_MIN	10
#define GP2Y0A21_DIST_MAX	80

#define GP2D120_DIST_MIN	4
#define GP2D120_DIST_MAX	30

uint8_t gp2d120_read(uint16_t adc);
uint8_t gp2y0a41_read(uint16_t adc);
uint8_t gp2y0a21_read(uint16_t adc);

uint8_t stop_robot(uint8_t *ir_ids, uint8_t ir_nb);

void analog_sensor_read(void);
void analog_sensor_setup(void);

#endif /* ANALOG_SENSOR_H_ */
