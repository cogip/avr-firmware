/*
 * analog_sensor.c
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "analog_sensor.h"
#include "log.h"
#include "sensor.h"

static volatile uint8_t distance[8];
static volatile uint8_t sensor_index;

static volatile uint8_t adc_flag;
static volatile uint16_t adc_result;

static void irq_adc_handler(void)
{
       adc_result = ADCA.CH0.RES;
       adc_flag = 1;
}

/*
 * @return distance in cm
 * */
uint8_t gp2d120_read(uint16_t adc)
{
	double voltage = adc * 3.3 / 255; /* 8-bits conversion - Vcc = 3.3V */
	/* double d = voltage * 0.0833 - 0.0016; // Vcc = 5V */
	double d = voltage * 0.126 - 0.007; /* Vcc = 3.3V */
	double distance = 1 / d;

	if ((distance < GP2D120_DIST_MIN) || (distance > GP2D120_DIST_MAX))
		distance = 0;

	return distance;
}

/*
 * @return distance in cm
 * */
uint8_t gp2y0a41_read(uint16_t adc)
{
	double voltage = adc * 3.3 / 255; /* 8-bits conversion - Vcc = 3.3V */
	/* double d = voltage * 0.0833 - 0.0016; // Vcc = 5V */
	double d = voltage * 0.1275 - 0.0253; /* Vcc = 3.3V */
	double distance = 1 / d;

	if ((distance < GP2Y0A41_DIST_MIN) || (distance > GP2Y0A41_DIST_MAX))
		distance = 0.0;

	return (uint8_t) distance;
}

/*
 * @return distance in cm
 * */
uint8_t gp2y0a21_read(uint16_t adc)
{
	double voltage = adc * 3.3 / 255; /* 8-bits conversion - Vcc = 3.3V */
	/*double d = voltage * 0.0833 - 0.0016; // Vcc = 5V */
	double d = voltage * 0.045 - 0.01; /* Vcc = 3.3V */
	double distance = 1 / d;

	if ((distance < GP2Y0A21_DIST_MIN) || (distance > GP2Y0A21_DIST_MAX))
		distance = 0.0;

	return (uint8_t) distance;
}

void analog_sensor_read(void)
{
	if (adc_flag) {
		distance[sensor_index] = gp2y0a21_read(adc_result);

		print_dbg("index = %d\tdist = %d\n",
			   sensor_index, distance[sensor_index]);

		sensor_index++;
		sensor_index %= 7;

		adc_read(&ADCA, sensor_index);

		adc_flag = 0;
	}
}

void analog_sensor_setup(void)
{
	adc_setup(&ADCA, irq_adc_handler);
}

/*
 * FIXME: following should be put elsewhere...
 */

uint8_t analog_sensor_detect_obstacle(uint8_t *ir_ids, uint8_t ir_nb)
{
	uint8_t stop = 0;
	uint8_t i;

	for (i = 0; i < ir_nb; i++) {
		if ((distance[ir_ids[i]] < 20) && (distance[ir_ids[i]] != 0)) {
			stop = 1;
		}
	}

	return stop;
}
