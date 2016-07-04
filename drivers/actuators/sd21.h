/**
 * \file sd21.h
 *
 * \date 10 fevr. 2015
 * \author ldo
 */

#ifndef SD21_H_
#define SD21_H_

#include <stdint.h>
#include <twi.h>

typedef struct {
	twi_t *twi;
	uint16_t twi_speed_khz;

	uint8_t servos_nb;
	struct {
		uint16_t value_init; /* pulse width in us */
	} servos[];
} sd21_t;

#define SD21_ADDRESS	(0xC2 >> 1)

#define REG_VERSION	64
#define REG_VOLTAGE	65

void sd21_setup(sd21_t *obj);
uint8_t sd21_version(sd21_t *obj);
double sd21_battery_voltage(sd21_t *obj);

/**
 * \fn void sd21_send (uint8_t servo, uint8_t speed, uint16_t position)
 * \brief
 * \param servo : servo number (to 1 from 21)
 * \param speed : servo speed (0 is the maximum speed)
 * \param position : pulse width in us
 */
void sd21_control_servo(uint8_t servo, uint8_t speed, uint16_t position);

#endif /* SD21_H_ */
