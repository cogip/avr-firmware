/*
 * analog_sensor.c
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */


#include "analog_sensor.h"
#include "log.h"


static void irq_adc_handler(uint16_t value, void *data)
{
	analog_sensors_t *as = (analog_sensors_t *)data;
	uint8_t i = as->sensor_index;

	/* save raw value in context */
	as->sensors[i].latest_raw_value = value;

	/* round robin acquisition using 1 ADC channel */
	as->sensor_index += 1;
	as->sensor_index %= as->sensors_nb;
	adc_async_read_start(as->adc, as->sensor_index);
}

void analog_sensor_setup(analog_sensors_t *as)
{
	if (as->sensors_nb) {
		adc_setup(as->adc, irq_adc_handler, as);

		/* start first conversion */
		adc_async_read_start(as->adc, 0);
	}
}

/*
 * FIXME: following should be put elsewhere...
 */

uint8_t analog_sensor_detect_obstacle(analog_sensors_t *as,
				      analog_sensor_zone_t zone)
{
	uint8_t i;
	uint8_t stop = 0;

	for (i = 0; i < as->sensors_nb; i++) {
		if ((as->sensors[i].zone & zone) &&
		     as->sensors[i].adc2cm_cb) {
			uint16_t raw = as->sensors[i].latest_raw_value;
			dist_cm_t dist = as->sensors[i].adc2cm_cb(raw);

			if (dist && dist < AS_DIST_LIMIT) {
				stop = 1;
				break;
			}
		}
	}

	return stop;
}
