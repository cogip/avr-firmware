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

/* Counter value is between [0..ENCODER_RES] at hardware stage,
 * Qdec driver shifts its zero reference to ENCODER_RES/2 (as it counter is
 * working on unsigned type) each time the counter is read.
 *
 * This function translate [0..ENCODER_RES/2] range to backward direction
 * all value in [ENCODER_RES/2..ENCODER_RES] are considered as forward
 * direction.
 */
static int16_t decode(const uint16_t counter, const uint8_t negate)
{
	int16_t signed_value = (int16_t) counter;

	signed_value -= WHEELS_ENCODER_RESOLUTION / 2;

	/* sense can be negated (ie. "polarity") */
	return negate ? -1 * signed_value : signed_value;
}

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	/* FIXME: -1 to be replaced by "polarity" in qdec structure */
	int16_t left_speed = decode(qdec_read(&encoders[0]), FALSE);
	int16_t right_speed = decode(qdec_read(&encoders[1]), TRUE);

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
