#include <avr/io.h>
#include <stdio.h>
#include <xmega/clksys.h>
#include <xmega/usart.h>

#include "controller.h"
#include "kos.h"
#include "log.h"
#include "platform.h"
#include "platform_task.h"
#include "sched.h"

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

#ifdef CONFIG_ANALOG_SENSORS
analog_sensors_t ana_sensors = {
	.adc = &ADCA,

	.sensors_nb = 8,
	.sensors = {
		[0] = {
			.pin_id = PIN0_bp,
			.adc2cm_cb = gp2y0a21_read,
			/* .zone = (AS_ZONE_LEFT | AS_ZONE_RIGHT), */
		},
		[1] = {
			.pin_id = PIN1_bp,
			.adc2cm_cb = gp2y0a21_read,
			/* .zone = (AS_ZONE_LEFT | AS_ZONE_RIGHT), */
		},
		[2] = {
			.pin_id = PIN2_bp,
			.adc2cm_cb = gp2y0a21_read,
			.zone = AS_ZONE_REAR,
		},
		[3] = {
			.pin_id = PIN3_bp,
			.adc2cm_cb = gp2y0a21_read,
			.zone = AS_ZONE_REAR,
		},
		[4] = {
			.pin_id = PIN4_bp,
			.adc2cm_cb = gp2y0a21_read,
			.zone = AS_ZONE_FRONT,
		},
		[5] = {
			.pin_id = PIN5_bp,
			.adc2cm_cb = gp2y0a21_read,
			.zone = AS_ZONE_FRONT,
		},
		[6] = {
			.pin_id = PIN6_bp,
			.adc2cm_cb = gp2y0a21_read,
			.zone = AS_ZONE_FRONT,
		},
		[7] = {
			.pin_id = PIN7_bp,
			.adc2cm_cb = gp2y0a21_read,
			.zone = AS_ZONE_FRONT,
		},
	}
};
#endif /* CONFIG_ANALOG_SENSORS */

qdec_t encoders[] = {
	{
		/* left motor */
		.pin_port = &PORTE,
		.pin_qdph0 = PIN4_bp,
		.pin_qdph90 = PIN5_bp,
		.event_channel = TC_EVSEL_CH0_gc,
		.tc = &TCE1,
		.line_count = WHEELS_ENCODER_RESOLUTION / 4,
		.polarity = 1,
	},
	{
		/* right motor */
		.pin_port = &PORTF,
		.pin_qdph0 = PIN0_bp,
		.pin_qdph90 = PIN1_bp,
		.event_channel = TC_EVSEL_CH2_gc,
		.tc = &TCF0,
		.line_count = WHEELS_ENCODER_RESOLUTION / 4,
		.polarity = -1,
	},
};

sd21_t sd21 = {
	.twi = &TWIC,
	.twi_speed_khz = 100,

	.servos_nb = 11,
	.servos = {
		/* [0...3] had no related action, not used then */
		/* Right arm */
		[4] = {
			.value_init = 2400,
			.value_open = 1200,
			.value_close = 2400,
		},
		/* Left arm */
		[5] = {
			.value_init = 600,
			.value_open = 1800,
			.value_close = 600,
		},
		/* Glasses right arm */
		[6] = {
			.value_init = 600,
			.value_open = 600,
			.value_close = 2450,
		},
		/* Glasses left arm */
		[7] = {
			.value_init = 2600,
			.value_open = 2600,
			.value_close = 800,
		},

		/* Clamp */
		[8] = {
			.value_init = 1700,
			.value_open = 2120,
			.value_close = 1700,
		},
		/* Right door */
		[9] = {
			.value_init = 1350,
			.value_open = 700,
			.value_close = 1350,
		},
		/* Left door */
		[10] = {
			.value_init = 875,
			.value_open = 1500,
			.value_close = 875,
		},
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

	.engine_nb = 3,
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


pose_t mach_trajectory_get_route_update(void)
{
	pose_t empty = {0,};
	return empty;
}

uint8_t mach_stop_robot(void)
{
	uint8_t stop = 0;
	return stop;
}


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

	/**
	 * PORTH and PORTJ as digital input
	 */
//	PORTH.DIR = 0x00;
//	PORTJ.DIRCLR = PIN0_bm;
//	PORTJ.DIRCLR = PIN1_bm;
}

#ifdef CONFIG_ENABLE_LOGGING
static void usartc0_putchar(const char c)
{
	usart_send(&USARTC0, c);
}

static int usartc0_getchar()
{
	return usart_recv(&USARTC0);
}
#endif

int mach_getchar_or_yield()
{
#ifdef CONFIG_ENABLE_LOGGING
	while (!usart_is_data_arrived(&USARTC0))
		kos_yield();
	return getchar();
#else
	kos_yield();
#endif
}

void mach_sched_init()
{
	sched_init(10/*ms*/, &TCC0);
}

void mach_sched_run()
{
	kos_run();
}

void mach_setup(void)
{
#if F_CPU == 32000000UL
	clksys_intrc_32MHz_setup();
#endif
	mach_pinmux_setup();

#ifdef CONFIG_ENABLE_LOGGING
	/* setup logs through usart communication */
	usart_setup(&USART_CONSOLE, 115200);
	log_init(usartc0_putchar, usartc0_getchar);
#endif

#ifdef CONFIG_ANALOG_SENSORS
	/* setup analog conversion */
	analog_sensor_setup(&ana_sensors);
#endif

	/* setup TWI communication with SD21 */
	sd21_setup(&sd21);

	//action_setup(); /* TODO: commenter pour debug */

	hbridge_setup(&hbridges);

	/* setup qdec */
	qdec_setup(&encoders[0]);
	qdec_setup(&encoders[1]);

	/* controller setup */
	odometry_setup(WHEELS_DISTANCE);
	controller_setup(WHEELS_DISTANCE);

	/* Programmable Multilevel Interrupt Controller */
	PMIC.CTRL |= PMIC_LOLVLEN_bm; /* Low-level Interrupt Enable */

	/* global interrupt enable */
	sei();
}
