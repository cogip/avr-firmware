/*
 * analog_sensor.c
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */


#include "analog_sensor.h"
#include "log.h"


static uint8_t adc_flag;
static uint16_t adc_result;

static void irq_adc_handler(uint16_t value)
{
	adc_result = value;
	adc_flag = 1;
}

void analog_sensor_read(analog_sensors_t *as)
{
	if (adc_flag) {
		uint8_t i = as->sensor_index;

		as->sensors[i].latest_dist = gp2y0a21_read(adc_result);

		print_dbg("index = %d\tdist = %d\n",
			   i, as->sensors[i].latest_dist);

		/* round robin acquisition using 1 ADC channel */
		as->sensor_index += 1;
		as->sensor_index %= as->sensors_nb;

		adc_async_read_start(as->adc, as->sensor_index);

		adc_flag = 0;
	}
}

void analog_sensor_setup(analog_sensors_t *as)
{
	adc_setup(as->adc, irq_adc_handler);

#if 0
	/* start first conversion */
	adc_async_read_start(as->adc, 0);
#endif
}

/*
 * FIXME: following should be put elsewhere...
 */

uint8_t analog_sensor_detect_obstacle(analog_sensors_t *as,
				      uint8_t *ir_ids,
				      uint8_t ir_nb)
{
	uint8_t i;
	uint8_t stop = 0;

	for (i = 0; i < ir_nb; i++) {
		if ((as->sensors[ir_ids[i]].latest_dist < 20) &&
		    (as->sensors[ir_ids[i]].latest_dist != 0)) {
			stop = 1;
			break;
		}
	}

	return stop;
}
