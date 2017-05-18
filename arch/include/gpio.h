#ifndef GPIO_H_
#define GPIO_H_

#if defined(__AVR__)
#include <avr/io.h>

typedef PORT_t gpio_port_t;
#else
#include <stdint.h>

typedef void gpio_port_t;

char _ports_[6];
#define PORTA (_ports_[0])
#define PORTB (_ports_[1])
#define PORTC (_ports_[2])
#define PORTD (_ports_[3])
#define PORTE (_ports_[4])
#define PORTF (_ports_[5])

#define PIN0_bp 0
#define PIN1_bp 1
#define PIN2_bp 2
#define PIN3_bp 3
#define PIN4_bp 4
#define PIN5_bp 5
#define PIN6_bp 6
#define PIN7_bp 7
#define PIN8_bp 8
#define PIN9_bp 9
#endif


#define GPIO_DIR_IN		0
#define GPIO_DIR_OUT		1

void gpio_set_direction(gpio_port_t *p, uint8_t pin_id, uint8_t output);
void gpio_set_output(gpio_port_t *p, uint8_t pin_id, int8_t value);
int8_t gpio_get_input(gpio_port_t *p, uint8_t pin_id);

#endif /* GPIO_H_ */
