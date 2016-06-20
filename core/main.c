/**
 * \file main.c
 *
 * \date 11 mars 2015
 * \author ldo
 */

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

#include <avr/interrupt.h>
#include <avr/io.h>

#include <xmega/timer.h>
#include <xmega/twi.h>
#include <xmega/usart.h>

#include "action.h"
#include "analog_sensor.h"
#include "controller.h"
#include "encoder.h"
#include "sd21.h"
#include "sensor.h"
#include "motor.h"
#include "odometry.h"
#include "route.h"

volatile uint8_t controller_flag;

polar_t	robot_speed;
polar_t	speed_setpoint		= { 60, 60 };
polar_t	motor_command;
pose_t	robot_pose		= { 1856.75, 0, 0 }; /* position absolue */
pose_t	pose_setpoint		= { 0, 0, 0 };
uint8_t	pose_reached;

int16_t	speed_elevator;
int16_t	distance_elevator;
int16_t	tempo;

static void clock_setup(void)
{
	/* Configuration change protection : Protected IO register
	 * disable automatically all interrupts for the next
	 * four CPU instruction cycles
	 */
	CCP = CCP_IOREG_gc;

	/* Oscillator : 32MHz Internal Oscillator Enable */
	OSC.CTRL = OSC_RC32MEN_bm;

	/* Wait for the internal 32 MHz RC oscillator to stabilize */
	while (!(OSC.STATUS & OSC_RC32MRDY_bm))
		;

	/* Configuration change protection : Protected IO register */
	CCP = CCP_IOREG_gc;

	/* Clock : 32MHz Internal Oscillator */
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}

static void interrupt_setup(void)
{
	/* Programmable Multilevel Interrupt Controller */
	PMIC.CTRL |= PMIC_LOLVLEN_bm; /* Low-level Interrupt Enable */

	/* global interrupt enable */
	sei();
}

static void pin_setup(void)
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
	PORTE.DIRSET = PIN0_bm;
	PORTE.DIRSET = PIN1_bm;
	PORTE.DIRSET = PIN2_bm;
	/* Configure PE4 and PE5 as input pin */
	PORTE.DIRCLR = PIN4_bm;
	PORTE.DIRCLR = PIN5_bm;
	/* A : QDPH0 - D0 */
	PORTE.PIN4CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;
	/* B : QDPH90 - D1 */
	PORTE.PIN5CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;

	/* PORTF encoder counter (PF0, PF1, PF4 and PF5) as input pin,
	 * Set QDPH0 and QDPH1 sensing level index not used here
	 */
	PORTF.DIRCLR = PIN0_bm;
	PORTF.DIRCLR = PIN1_bm;
	/* A : QDPH0 - D0 */
	PORTF.PIN0CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;
	/* B : QDPH90 - D1 */
	PORTF.PIN1CTRL = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;
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

/*
 * setup quadrature decoder A & B (index is not used here)
 * use CH0, CH2 and CH4
 */
void qdec_setup(void)
{
	/* Configure event channel x assign to pin x */
	/* A & B inputs to quad-decoder */
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTE_PIN4_gc;
	EVSYS.CH0CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;
	xmega_timer_1_qdec_mode_setup(&TCE1, TC_EVSEL_CH0_gc, 500);

	/* Configure event channel x assign to pin x */
	/* A & B inputs to quad-decoder */
	EVSYS.CH2MUX = EVSYS_CHMUX_PORTF_PIN0_gc;
	EVSYS.CH2CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;

	xmega_timer_0_qdec_mode_setup(&TCF0, TC_EVSEL_CH2_gc, 500);
}

/* Timer 0 Overflow interrupt */
ISR(TCC0_OVF_vect)
{
	controller_flag = 1;
}

/**
 *
 */
static void setup(void)
{
	clock_setup();
	pin_setup();
	interrupt_setup();

	/* timer setup */
	controller_flag = 0;
	xmega_timer_0_normal_mode_setup(&TCC0, 625, TC_CLKSEL_DIV1024_gc);

	/* setup usart communication */
	xmega_usart_setup(&USARTC0);

	/* setup TWI communication with SD21 */
	sd21_setup(&TWIC);

	action_setup(); /* TODO: commenter pour debug */

	/* setup frequency waveform generation (PWM) */
	/* note: 200 pour 20KHz -100 pour pour 40 KHz */
	xmega_timer_0_pwm_mode_setup(&TCE0, 200, TC_CLKSEL_DIV8_gc);
	xmega_timer_0_pwm_enable(&TCE0, 0); /* PE0 */
	xmega_timer_0_pwm_enable(&TCE0, 1); /* PE1 */
	xmega_timer_0_pwm_enable(&TCE0, 2); /* PE2 */

	/* setup qdec */
	qdec_setup();

	/* setup analog conversion */
	xmega_adc_setup(&ADCA);

	/* controller setup */
	odometry_setup(WHEELS_DISTANCE);
	controller_setup();
}

/**
 * TODO 90 s
 * TODO gestion bras maintien position plus fermé
 * TODO init trappe pop corn
 */
int main(void)
{
	uint8_t all_irs[] = { 2, 3, 4, 5, 6, 7 };
	uint8_t stop = 0;

	setup();
	/* start first conversion */
	xmega_adc_read(&ADCA, 0);
	xmega_usart_transmit(&USARTC0, 0xAA);

	while (detect_start())
		gestion_tour();

	/* main loop */
	while (tempo < 4500) {
		if (controller_flag == 1) {
			/* we enter here every 20ms */
			tempo++;

			/* catch speed */
			robot_speed = read_encoder();

			/* convert to position */
			odometry_update(&robot_pose, &robot_speed, SEGMENT);

			/* get next pose_t to reach */
			pose_setpoint = route_update();

			pose_setpoint.x *= PULSE_PER_MM;
			pose_setpoint.y *= PULSE_PER_MM;
			pose_setpoint.O *= PULSE_PER_DEGREE;

			/* mirror mode: invert path regarding bot's camp */
			if (detect_color()) {
				pose_setpoint.y *= -1;
				pose_setpoint.O *= -1;
			}

			/* collision detect */

#if 0
			uint8_t side_irs[] = { 0, 1 };
			uint8_t rear_irs[] = { 2, 3 };
			uint8_t front_irs[] = { 4, 5, 6, 7 };

			if ((right_command > 0) && (left_command > 0))
			stop = stop_robot(front_irs, 4);
			else if ((right_command < 0) && (left_command < 0))
			stop = stop_robot(rear_irs, 2);
			else
#endif
			stop = stop_robot(all_irs, 6);

			if (stop && get_route_index()) {
				speed_setpoint.distance = 0;
				speed_setpoint.angle = 0;
#if 0
				if ((stop > 1) && (get_can_retro()))
				down_route_index();
#endif
			} else {
				speed_setpoint.distance = 60;
				speed_setpoint.angle = 60;
			}

			/* PID / feedback control */
#if 0
			motor_command = speed_controller(speed_setpoint,
							 robot_speed);
#endif
			motor_command = controller_update(pose_setpoint,
							  robot_pose,
							  speed_setpoint,
							  robot_speed);

			/* set speed to wheels */
			motor_drive(motor_command);

			controller_flag = 0;
		}

		attraper_cup();
		read_analog_sensor();
		gestion_tour();
	}

	/* final position */
	while (1) {
		open_pince();
		open_door();
		set_release_right_cup();
		set_release_left_cup();
		motor_command.distance = 0;
		motor_command.angle = 0;
		motor_drive(motor_command);
	}

	/* we never reach this point */
	return 0;
}
