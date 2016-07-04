/**
 * \file sd21.c
 *
 * \date 10 fevr. 2015
 * \author ldo
 *
 * \brief sd21 library use twi library
 */

#include "sd21.h"

/**
 */
void sd21_setup(sd21_t *obj)
{
	uint8_t i;

	twi_master_setup(obj->twi, obj->twi_speed_khz);

	for (i = 0; i < obj->servos_nb; i++) {
		uint16_t value_init = obj->servos[i].value_init;

		if (value_init)
			sd21_control_servo(/* obj, */i, 0, value_init);
	}
}

/**
 * Register 64 is the software revision number
 */
uint8_t sd21_version(sd21_t *obj)
{
	uint8_t reg = REG_VERSION;
	uint8_t data = 0x00;

	twi_read(obj->twi, SD21_ADDRESS, &reg, &data, 1);

	return data;
}

/**
 * Register 65 contains the servo battery voltage in 39mV units up to
 * a maximum of 10V.
 */
double sd21_battery_voltage(sd21_t *obj)
{
	uint8_t reg = REG_VOLTAGE;
	uint8_t data = 0x00;

	twi_read(obj->twi, SD21_ADDRESS, &reg, &data, 1);

	double voltage = data * 0.039;

	return voltage;
}

/**
 * Blue : min = 800 - max = 2400
 * Tower Pro : min = 600 - max = 2400
 * Emax : min = 600 - max = 2600
 */
static sd21_t *singleton;
void sd21_control_servo(/*sd21_t *obj, */uint8_t servo, uint8_t speed, uint16_t position)
{
	uint8_t reg = (servo) * 3;
	uint8_t data[2];

	data[0] = reg;
	data[1] = speed;
	twi_write(singleton->twi, SD21_ADDRESS, data, 2);

	data[0] = reg + 1;
	data[1] = (uint8_t) (position & 0x00ff);
	twi_write(singleton->twi, SD21_ADDRESS, data, 2);

	data[0] = reg + 2;
	data[1] = position >> 8;
	twi_write(singleton->twi, SD21_ADDRESS, data, 2);
}
