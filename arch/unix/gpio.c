#include "gpio.h"

#include "platform.h"

extern int8_t simu_left_motor_dir;
extern int8_t simu_right_motor_dir;

inline void gpio_set_direction(gpio_port_t *p, uint8_t pin_id, uint8_t output)
{
//	if (output)
//		p->DIRSET = (1 << pin_id);
//	else
//		p->DIRCLR = (1 << pin_id);
}

inline void gpio_set_output(gpio_port_t *p, uint8_t pin_id, int8_t value)
{
//	if (value)
//		p->OUT |= (1 << pin_id);
//	else
//		p->OUT &= ~(1 << pin_id);
	if (p == hbridges.engines[HBRIDGE_MOTOR_LEFT].direction_pin_port
	    && pin_id == hbridges.engines[HBRIDGE_MOTOR_LEFT].direction_pin_id)
		simu_left_motor_dir = value;

	if (p == hbridges.engines[HBRIDGE_MOTOR_RIGHT].direction_pin_port
	    && pin_id == hbridges.engines[HBRIDGE_MOTOR_RIGHT].direction_pin_id)
		simu_right_motor_dir = value;
}

inline int8_t gpio_get_input(gpio_port_t *p, uint8_t pin_id)
{
	return 1;
}
