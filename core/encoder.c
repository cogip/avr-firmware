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

/* Counter value is between [0..ENCODER_RES]
 * This function translate [0..ENCODER_RES/2] range to forward direction
 * all value in [ENCODER_RES/2..ENCODER_RES] are considered as backward
 * direction, thus shifted as below:
 */
static int16_t decode(int16_t counter)
{
	if (counter > (WHEELS_ENCODER_RESOLUTION / 2))
		counter -= WHEELS_ENCODER_RESOLUTION;

#if 0
	/* What the hell is going on here? */
	if (counter < -(WHEELS_ENCODER_RESOLUTION / 2))
		counter += WHEELS_ENCODER_RESOLUTION;
#endif

	return counter;
}

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	/* FIXME: -1 to be replaced by "polarity" in qdec structure */
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
