/**
 * \file sd21.h
 *
 * \date 10 fevr. 2015
 * \author ldo
 */

#ifndef SD21_H_
#define SD21_H_

#include <avr/io.h>
//#include "xmega_twi.h"
#include <xmega/twi.h>

#define SD21_ADDRESS 0xC2 >> 1

#define REG_VERSION 64
#define REG_VOLTAGE 65

/**
 * \fn void sd21_setup (TWI_t *twi)
 * \brief setup twi port
 * \param twi twi port
 */
void
sd21_setup (TWI_t *twi);

/**
 * \fn uint8_t sd21_version (void)
 * \brief
 * \return software revision number
 */
uint8_t
sd21_version (void);

/**
 * \fn double sd21_battery_voltage (void)
 * \brief
 * \return servo battery voltage [V]
 */
double
sd21_battery_voltage (void);

/**
 * \fn void sd21_send (uint8_t servo, uint8_t speed, uint16_t position)
 * \brief
 * \param servo : servo number (to 1 from 21)
 * \param speed : servo speed (0 is the maximum speed)
 * \param position : pulse width in us
 */
void
sd21_control_servo (uint8_t servo, uint8_t speed, uint16_t position);

#endif /* SD21_H_ */
