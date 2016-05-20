/*
 * analog_sensor.h
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */

#ifndef ANALOG_SENSOR_H_
#define ANALOG_SENSOR_H_

#include <avr/io.h>
#include <avr/interrupt.h>

//#include <xmega/usart/xmega_usart.h>
#include <xmega/adc.h>

#define GP2Y0A41_DIST_MIN 4
#define GP2Y0A41_DIST_MAX 30

#define GP2Y0A21_DIST_MIN 10
#define GP2Y0A21_DIST_MAX 80

#define GP2D120_DIST_MIN 4
#define GP2D120_DIST_MAX 30

uint8_t
gp2d120_read (uint16_t adc);
uint8_t
gp2y0a41_read (uint16_t adc);
uint8_t
gp2y0a21_read (uint16_t adc);

void
read_analog_sensor (void/*uint16_t adc_result*/);

uint8_t
detect_obstacle (uint8_t *ir_ids, uint8_t ir_nb);

#endif /* ANALOG_SENSOR_H_ */
