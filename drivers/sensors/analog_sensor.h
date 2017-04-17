#ifndef ANALOG_SENSOR_H_
#define ANALOG_SENSOR_H_

#include <adc.h>

#if defined(CONFIG_GP2D120)
#include "gp2d120.h"
#endif
#if defined(CONFIG_GP2Y0A21)
#include "gp2y0a21.h"
#endif
#if defined(CONFIG_GP2Y0A41)
#include "gp2y0a41.h"
#endif

/* from ADC value to distance in centimeters */
typedef uint8_t (* func_conv_t)(uint16_t);

typedef uint8_t dist_cm_t;

typedef uint16_t analog_sensor_zone_t;
#define AS_ZONE_FRONT	0x0001
#define AS_ZONE_REAR	0x0002
#define AS_ZONE_LEFT	0x0004
#define AS_ZONE_RIGHT	0x0008
#define AS_ZONE_ALL	0x000F
#define AS_ZONE_OTHER	0x0010

#define AS_DIST_LIMIT	20 /*cm*/

/* Average measurement over time */
#define ANALOG_SENSOR_NB_SAMPLES	3

typedef struct {
	adc_t *adc;

	uint8_t sensor_index;  /* current sensor in acquisition */
	uint8_t sensors_nb;
	struct {
		uint8_t pin_id;
		func_conv_t adc2cm_cb;
		analog_sensor_zone_t zone;

		/* acquisition context */
		uint16_t raw_values[ANALOG_SENSOR_NB_SAMPLES]; /* keep acquired distances */
	} sensors[];
} analog_sensors_t;

void analog_sensor_refresh_all(analog_sensors_t *as);
void analog_sensor_setup(analog_sensors_t *as);

uint8_t analog_sensor_detect_obstacle(analog_sensors_t *as,
				      analog_sensor_zone_t zone);

#if defined(CONFIG_CALIBRATION)
void analog_sensor_enter_calibration(analog_sensors_t *obj);
#endif

#endif /* ANALOG_SENSOR_H_ */
