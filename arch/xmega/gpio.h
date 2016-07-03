#ifndef GPIO_H_
#define GPIO_H_

#include <avr/io.h>

#define GPIO_DIR_IN		0
#define GPIO_DIR_OUT		1

typedef PORT_t gpio_port_t;

inline void gpio_set_direction(gpio_port_t *p, uint8_t pin_id, uint8_t output);
inline void gpio_set_output(gpio_port_t *p, uint8_t pin_id, int8_t value);

#endif /* GPIO_H_ */
