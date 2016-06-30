#include <avr/io.h>
#include "platform.h"

qdec_t encoders[] = {
	{
		/* left motor */
		.pin_port = &PORTE,
		.pin_qdph0 = PIN4_bp,
		.pin_qdph90 = PIN5_bp,
		.event_channel = TC_EVSEL_CH0_gc,
		.tc = &TCE1,
		.line_count = WHEELS_ENCODER_RESOLUTION / 4,
	},
	{
		/* right motor */
		.pin_port = &PORTF,
		.pin_qdph0 = PIN0_bp,
		.pin_qdph90 = PIN1_bp,
		.event_channel = TC_EVSEL_CH2_gc,
		.tc = &TCF0,
		.line_count = WHEELS_ENCODER_RESOLUTION / 4,
	},
};

/* TCE0 ClkIn == ClkPer / 8 == 4000 KHz */
/* Counter set to 200 for 20KHz output */
#define TCE0_MOTOR_PRESCALER		TC_CLKSEL_DIV8_gc
#define TCE0_MOTOR_PER_VALUE		200

hbridge_t hbridges = {
	.tc = &TCE0,
	.period = TCE0_MOTOR_PER_VALUE,
	.prescaler = TCE0_MOTOR_PRESCALER,

	.pwm_port = &PORTE, /* TODO: can be 'guessed' from timer ref above */

	.engine_nb = 2,
	.engines = {
		[HBRIDGE_MOTOR_LEFT] = {
			/* left motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN4_bm,
			.pwm_channel = PIN0_bp,
		},
		[HBRIDGE_MOTOR_RIGHT] = {
			/* right motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN5_bm,
			.pwm_channel = PIN1_bp,
		},
	},
};

void mach_pinmux_setup(void)
{
	/* analog to digital conversion */
	PORTA.DIR = 0x00; /*!< PORTA as input pin */
	PORTA.OUT = 0x00;
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;

	/* twi configuration pin */
	PORTC.DIRSET = PIN1_bm; /*!< PC1 (SCL) as output pin */
	/* usart configuration pin */
	PORTC.DIRCLR = PIN2_bm; /*!< PC2 (RDX0) as input pin */
	PORTC.DIRSET = PIN3_bm; /*!< PC3 (TXD0) as output pin */

	/* Configure PD4, PD5 and PD6 as output pin */
	PORTD.DIR = 0xFF;

	/* Configure PE0, PE1 and PE2 as output pin */
	PORTE.DIRSET = PIN2_bm;

	/* PORTF encoder counter (PF4 and PF5) as input pin,
	 * Set QDPH0 and QDPH1 sensing level index not used here
	 */
	PORTF.DIRCLR = PIN4_bm;
	PORTF.DIRCLR = PIN5_bm;
	/* A : QDPH0 - D0 */
	PORTF.PIN4CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;
	/* B : QDPH90 - D1 */
	PORTF.PIN5CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;

	/**
	 * PORTH and PORTJ as digital input
	 */
	PORTH.DIR = 0x00;
	PORTJ.DIRCLR = PIN0_bm;
	PORTJ.DIRCLR = PIN1_bm;
}
