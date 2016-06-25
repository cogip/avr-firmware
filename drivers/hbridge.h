#ifndef HBRIDGE_H_
#define HBRIDGE_H_

#include <avr/io.h>

typedef struct {
	PORT_t *direction_pin_port;
	uint8_t direction_pin_id;
	uint8_t pwm_channel;
} hbridge_t;

void hbridge_update(hbridge_t *b, int16_t pwm);

#endif /* HBRIDGE_H_ */
