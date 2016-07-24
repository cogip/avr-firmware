#include "gpio.h"

inline void gpio_set_direction(gpio_port_t *p, uint8_t pin_id, uint8_t output)
{
	if (output)
		p->DIRSET = (1 << pin_id);
	else
		p->DIRCLR = (1 << pin_id);
}

inline void gpio_set_output(gpio_port_t *p, uint8_t pin_id, int8_t value)
{
	if (value)
		p->OUT |= (1 << pin_id);
	else
		p->OUT &= ~(1 << pin_id);
}
