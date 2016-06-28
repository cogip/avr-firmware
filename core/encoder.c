/*
 * encoder.c
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#include <avr/io.h>
#include <xmega/timer.h>

#include "encoder.h"

#define LINE_COUNT		500

/* FIXME: some part of this file should be moved to arch/xmega */

/*
 * setup quadrature decoder A & B (index is not used here)
 * use CH0, CH2 and CH4
 */
void encoder_setup(void)
{
	/* Configure PE4 and PE5 as input pin */
	PORTE.DIRCLR = PIN4_bm;
	PORTE.DIRCLR = PIN5_bm;
	/* A : QDPH0 - D0 */
	PORTE.PIN4CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;
	/* B : QDPH90 - D1 */
	PORTE.PIN5CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;

	/* PORTF encoder counter (PF0, PF1) as input pin,
	 * Set QDPH0 and QDPH1 sensing level index not used here
	 */
	PORTF.DIRCLR = PIN0_bm;
	PORTF.DIRCLR = PIN1_bm;
	/* A : QDPH0 - D0 */
	PORTF.PIN0CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;
	/* B : QDPH90 - D1 */
	PORTF.PIN1CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;


	/* Configure event channel x assign to pin x */
	/* A & B inputs to quad-decoder */
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTE_PIN4_gc;
	EVSYS.CH0CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;
	timer_qdec_mode_setup(&TCE1, TC_EVSEL_CH0_gc, LINE_COUNT);

	/* Configure event channel x assign to pin x */
	/* A & B inputs to quad-decoder */
	EVSYS.CH2MUX = EVSYS_CHMUX_PORTF_PIN0_gc;
	EVSYS.CH2CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;

	timer_qdec_mode_setup(&TCF0, TC_EVSEL_CH2_gc, LINE_COUNT);
}

static int16_t decode(int16_t counter)
{
	if (counter > (LINE_COUNT * 2))
		counter -= LINE_COUNT * 4;

	if (counter < -(LINE_COUNT * 2))
		counter += LINE_COUNT * 4;

	return counter;
}

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = decode(TCE1.CNT);
	int16_t right_speed = decode(-TCF0.CNT);

#if 0
	xmega_usart_transmit(&USARTC0, (int8_t) (left_speed >> 8));
	xmega_usart_transmit(&USARTC0, (int8_t) left_speed);
	xmega_usart_transmit(&USARTC0, (int8_t) (right_speed >> 8));
	xmega_usart_transmit(&USARTC0, (int8_t) right_speed);
#endif

	TCE1.CNT = 0;
	TCF0.CNT = 0;

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
