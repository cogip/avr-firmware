/*
 * analog_sensor.c
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "analog_sensor.h"

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

void read_analog_sensor(void)
{
	if (adc_flag) {
		/*xmega_usart_transmit (&USARTC0, 0xAA);
		xmega_usart_transmit (&USARTC0, 0xAA);
		xmega_usart_transmit (&USARTC0, 0x00);
		xmega_usart_transmit (&USARTC0, distance[7][value_index]);*/
		distance[sensor_index] = gp2y0a21_read(adc_result);

		sensor_index++;
		if (sensor_index > 7)
			sensor_index = 0;

		adc_read(&ADCA, sensor_index);

		adc_flag = 0;
	}
}

uint8_t detect_obstacle(uint8_t *ir_ids, uint8_t ir_nb)
{
	uint8_t stop = 0;
	uint8_t i;

	for (i = 0; i < ir_nb; i++) {
		if ((distance[ir_ids[i]] < 20) && (distance[ir_ids[i]] != 0)) {
		  stop = 1;
		  /*xmega_usart_transmit (&USARTC0, 0xCC);
		   xmega_usart_transmit (&USARTC0, 0xCC);
		   xmega_usart_transmit (&USARTC0, i);
		   xmega_usart_transmit (&USARTC0, distance_filtered);*/
		}
	}

	return stop;
}

void analog_sensor_setup(void)
{
	adc_setup(&ADCA, irq_adc_handler);
}
