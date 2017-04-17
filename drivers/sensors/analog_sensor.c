#include "analog_sensor.h"
#include "console.h"
#include "kos.h"
#include "log.h"
#include "utils.h"

#ifdef CONFIG_CALIBRATION
static int8_t display_dbg = FALSE;
#endif

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

#if defined(CONFIG_CALIBRATION)
static void analog_sensor_dump_all(analog_sensors_t *as)
{
	uint8_t i;

	cons_printf("\n");
	for (i = 0; i < as->sensors_nb; i++) {
		uint16_t raw = as->sensors[i].latest_raw_value;
		dist_cm_t dist;

		cons_printf("\ti=%d\tpin=%d %d", i, as->sensors[i].pin_id, raw);

		if (as->sensors[i].adc2cm_cb) {
			dist = as->sensors[i].adc2cm_cb(raw);
			printf("\t%3d", dist);
		}

		cons_printf("\n");
	}
}

static void analog_sensor_calibration_usage(analog_sensors_t *obj)
{
	cons_printf("\n>>> Entering analog sensor calibration\n\n");

	cons_printf("sensors_nb = %d\n\n", obj->sensors_nb);
	analog_sensor_dump_all(obj);

	cons_printf("\n");
	cons_printf("\t'S' to start/stop measurements dumps\n");
	cons_printf("\n");
	cons_printf("\t'h' to display this help\n");
	cons_printf("\t'q' to quit\n");
	cons_printf("\n");
}

void analog_sensor_enter_calibration(analog_sensors_t *obj)
{
	int c = -1;
	uint8_t quit = 0;

	analog_sensor_calibration_usage(obj);

	while (!quit) {
		if (! display_dbg) {
			/* display prompt */
			cons_printf("$ ");

			/* wait for command, or schedule */
			c = cons_getchar();
			cons_printf("%c\n", c);
		} else {
			/* poll for command if arrived, or dump values
			 * periodically */
			kos_set_next_schedule_delay_ms(250);

			if (cons_is_data_arrived())
				c = cons_getchar();
			else
				c = -1;

			analog_sensor_dump_all(obj);
		}

		switch (c) {
		case -1:
			kos_yield();
			break;
		case 'S':
			display_dbg = !display_dbg;

			cons_printf("display_dbg = %s\n", display_dbg ? "True" : "False");
			break;
		case 'h':
			analog_sensor_calibration_usage(obj);
			break;
		case 'q':
			quit = 1;
			break;
		default:
			cons_printf("\n");
			break;
		}
	}
}
#endif /* CONFIG_CALIBRATION */
