#include "gp2y0a21.h"

#define GP2Y0A21_DIST_MIN	10
#define GP2Y0A21_DIST_MAX	80

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
