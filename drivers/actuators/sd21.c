/**
 * \file sd21.c
 *
 * \date 10 fevr. 2015
 * \author ldo
 *
 * \brief sd21 library use twi library
 */

#include "log.h"
#include "sd21.h"

#define SD21_ADDRESS	(0xC2 >> 1)

#define REG_VERSION	64
#define REG_VOLTAGE	65

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
/**
 * \fn void sd21_send (uint8_t servo, uint8_t speed, uint16_t position)
 * \brief
 * \param servo : servo number (to 1 from 21)
 * \param speed : servo speed (0 is the maximum speed)
 * \param position : pulse width in us
 */
static void sd21_send_twi_cmd(twi_t *twi, uint8_t servo, uint8_t speed, uint16_t position)
{
	uint8_t reg = (servo) * 3;
	uint8_t data[2];

	data[0] = reg;
	data[1] = speed;
	twi_write(twi, SD21_ADDRESS, data, 2);

	data[0] = reg + 1;
	data[1] = (uint8_t) (position & 0x00ff);
	twi_write(twi, SD21_ADDRESS, data, 2);

	data[0] = reg + 2;
	data[1] = position >> 8;
	twi_write(twi, SD21_ADDRESS, data, 2);
}

void sd21_control_servo(sd21_t * obj, uint8_t servo_id, uint8_t position)
{
	uint16_t value;

	switch(position) {
	case SD21_SERVO_OPEN:
		value = obj->servos[servo_id].value_open;
		break;
	case SD21_SERVO_CLOSE:
		value = obj->servos[servo_id].value_close;
		break;
	default:
		print_err("Invalid position\n");
		return;
	}

	sd21_send_twi_cmd(obj->twi, servo_id, 0, value);
}

/**
 */
void sd21_setup(sd21_t *obj)
{
	uint8_t i;

	twi_master_setup(obj->twi, obj->twi_speed_khz);

	for (i = 0; i < obj->servos_nb; i++) {
		uint16_t value_init = obj->servos[i].value_init;

		if (value_init)
			sd21_send_twi_cmd(obj->twi, i, 0, value_init);
	}
}
