#include "gp2d120.h"

#define GP2D120_DIST_MIN	4
#define GP2D120_DIST_MAX	30

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
