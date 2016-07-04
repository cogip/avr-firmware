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

/* from ADC value to distance in centimeters */
typedef uint8_t (* func_conv_t)(uint16_t);

typedef uint8_t dist_cm_t;

typedef struct {
	adc_t *adc;

	uint8_t sensor_index;  /* current sensor in acquisition */
	uint8_t sensors_nb;
	struct {
		uint8_t pin_id;
		func_conv_t adc2cm_cb;

		/* acquisition context */
		dist_cm_t latest_dist; /* keep acquired distances */
	} sensors[];
} analog_sensors_t;

void analog_sensor_read(analog_sensors_t *as);
void analog_sensor_setup(analog_sensors_t *as);

uint8_t analog_sensor_detect_obstacle(analog_sensors_t *as,
				      uint8_t *ir_ids,
				      uint8_t ir_nb);

#endif /* ANALOG_SENSOR_H_ */
