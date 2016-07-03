/*
 * analog_sensor.h
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */

#ifndef ANALOG_SENSOR_H_
#define ANALOG_SENSOR_H_

#include <adc.h>

#define GP2Y0A41_DIST_MIN	4
#define GP2Y0A41_DIST_MAX	30

#define GP2Y0A21_DIST_MIN	10
#define GP2Y0A21_DIST_MAX	80

#define GP2D120_DIST_MIN	4
#define GP2D120_DIST_MAX	30

uint8_t gp2d120_read(uint16_t adc);
uint8_t gp2y0a41_read(uint16_t adc);
uint8_t gp2y0a21_read(uint16_t adc);


typedef struct {
	ADC_t *adc;

} analog_sensors_t;

void analog_sensor_read(analog_sensors_t *as);
void analog_sensor_setup(analog_sensors_t *as);

uint8_t analog_sensor_detect_obstacle(uint8_t *ir_ids, uint8_t ir_nb);

#endif /* ANALOG_SENSOR_H_ */
