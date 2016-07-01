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

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = qdec_read(&encoders[0]);
	int16_t right_speed = qdec_read(&encoders[1]);

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
