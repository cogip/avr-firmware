/**
 * \file sd21.c
 *
 * \date 10 fevr. 2015
 * \author ldo
 *
 * \brief sd21 library use xmega_twi library
 */

#include "sd21.h"

TWI_t *sd21_twi;

/**
 */
void sd21_setup(TWI_t *twi)
{
	sd21_twi = twi;
	xmega_twi_master_setup(twi, 100);
}

/**
 * Register 64 is the software revision number
 */
uint8_t sd21_version(void)
{
	uint8_t reg = REG_VERSION;
	uint8_t data = 0x00;

	xmega_twi_read(sd21_twi, SD21_ADDRESS, &reg, &data, 1);

	return data;
}

/**
 * Register 65 contains the servo battery voltage in 39mV units up to
 * a maximum of 10V.
 */
double sd21_battery_voltage(void)
{
	uint8_t reg = REG_VOLTAGE;
	uint8_t data = 0x00;

	xmega_twi_read(sd21_twi, SD21_ADDRESS, &reg, &data, 1);

	double voltage = data * 0.039;

	return voltage;
}

/**
 * Blue : min = 800 - max = 2400
 * Tower Pro : min = 600 - max = 2400
 * Emax : min = 600 - max = 2600
 */
void sd21_control_servo(uint8_t servo, uint8_t speed, uint16_t position)
{
	uint8_t reg = (servo - 1) * 3;
	uint8_t data[2];

	data[0] = reg;
	data[1] = speed;
	xmega_twi_write(sd21_twi, SD21_ADDRESS, data, 2);

	data[0] = reg + 1;
	data[1] = (uint8_t) position;
	xmega_twi_write(sd21_twi, SD21_ADDRESS, data, 2);

	data[0] = reg + 2;
	data[1] = position >> 8;
	xmega_twi_write(sd21_twi, SD21_ADDRESS, data, 2);
}
