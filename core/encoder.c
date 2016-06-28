/*
 * encoder.c
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#include <avr/io.h>
#include <xmega/qdec.h>

#include "encoder.h"
#include "platform.h"

/*
 * setup quadrature decoder A & B (index is not used here)
 * use CH0, CH2 and CH4
 */
void encoder_setup(void)
{
	qdec_setup(&encoders[0]);
	qdec_setup(&encoders[1]);
}

/* FIXME: should we move this to qdec? */
static int16_t decode(int16_t counter)
{
	if (counter > (QDEC_LINE_COUNT * 2))
		counter -= QDEC_LINE_COUNT * 4;

	if (counter < -(QDEC_LINE_COUNT * 2))
		counter += QDEC_LINE_COUNT * 4;

	return counter;
}

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = decode(qdec_read(&encoders[0]));
	int16_t right_speed = decode(-1 * qdec_read(&encoders[1]));

#if 0
	xmega_usart_transmit(&USARTC0, (int8_t) (left_speed >> 8));
	xmega_usart_transmit(&USARTC0, (int8_t) left_speed);
	xmega_usart_transmit(&USARTC0, (int8_t) (right_speed >> 8));
	xmega_usart_transmit(&USARTC0, (int8_t) right_speed);
#endif

	/* update speed */
	robot_speed.distance = (right_speed + left_speed) / 2.0;
	robot_speed.angle = right_speed - left_speed;

#if 0
	xmega_usart_transmit(&USARTC0, (int8_t) (robot_speed.distance >> 8));
	xmega_usart_transmit(&USARTC0, (int8_t) robot_speed.distance);
	xmega_usart_transmit(&USARTC0, (int8_t) (robot_speed.angle >> 8));
	xmega_usart_transmit(&USARTC0, (int8_t) robot_speed.angle);
#endif

	return robot_speed;
}
