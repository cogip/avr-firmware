#include <avr/io.h>

#include <xmega/clksys.h>
#include <xmega/usart.h>

#include "action.h"
#include "analog_sensor.h"
#include "encoder.h"
#include "platform.h"
#include "sd21.h"

/**
 * PORTA : ANA input
 *	PA0 - PA7 : IR
 * PORTB : JTAG + ANA inputs
 * PORTC : Communication
 *	PC0 (SDA) : i2c communication
 *	PC1 (SCL) : i2c communication
 *	PC2 (RX) : Debug communication
 *	PC3 (TX) :
 *	PC6 (RX1) : (Lidar communication)
 *	PC7 (TX1) :
 * PORTD : Communication
 *	PD2 (RX2) : (MotherBoard communication)
 *	PD3 (TX2) :
 *	PD4 : DIR1 left motor
 *	PD5 : DIR2 right motor
 *	PD6 : DIR3 elevator motor
 * PORTE : Timer + PWM
 *	PE0 (OC0A) : PWM1 left motor
 *	PE1 (OC0B) : PWM2 right motor
 *	PE2 (OC0C) : PWM3
 *	PE3 (OC0D) : -
 *	PE4 : encoder A1 left wheel
 *	PE5 : encoder B1
 * PORTF : Timer decoder quadrature
 *	PF0 : encoder A2 right wheel
 *	PF1 : encoder B2
 *	PF4 : encoder A3
 *	PF5 : encoder B3
 * PORTH : Digital port
 *	PH0 : capteur TOR 19
 *	PH1 : 18
 *	PH2 : 17
 * PORTJ :
 * PORTK :
 * PORTQ : TOSC
 * PORTR : PDI + XTAL
 *
 * use TCC0 as general timer
 * use TCE0 timer to generate PWM signal
 * use TCE1, TCF0 and TCF1 timers to decode quadrature
 */

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
		[HBRIDGE_MOTOR_TOWER] = {
			/* tower motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN6_bm,
			.pwm_channel = PIN2_bp,
		},
	},
};

static void mach_pinmux_setup(void)
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

extern uint8_t next_timeslot_trigged;

/* Timer 0 Overflow interrupt */
static void irq_timer_tcc0_handler(void)
{
	next_timeslot_trigged = 1;
}

void mach_setup(void)
{
#if F_CPU == 32000000UL
	clksys_intrc_32MHz_setup();
#endif
	mach_pinmux_setup();

	/* setup analog conversion */
	analog_sensor_setup();

	/* timer setup */
	next_timeslot_trigged = 0;
	timer_0_register_ovf_cb(irq_timer_tcc0_handler);

	/* TCC0 ClkIn == ClkPer / 1024 == 31.25 KHz */
	/* Counter set to 625 for 50Hz output (20ms) */
	timer_normal_mode_setup(&TCC0, 625, TC_CLKSEL_DIV1024_gc);

	/* setup usart communication */
	xmega_usart_setup(&USARTC0);

	/* setup TWI communication with SD21 */
	sd21_setup(&TWIC);

	action_setup(); /* TODO: commenter pour debug */

	hbridge_setup(&hbridges);

	/* setup qdec */
	qdec_setup(&encoders[0]);
	qdec_setup(&encoders[1]);

	/* Programmable Multilevel Interrupt Controller */
	PMIC.CTRL |= PMIC_LOLVLEN_bm; /* Low-level Interrupt Enable */

	/* global interrupt enable */
	sei();
}
