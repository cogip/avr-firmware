#include <stdio.h>

#include "clksys.h"
#include "gpio.h"
#include "usart.h"

#include "console.h"
#include "kos.h"
#include "msched.h"
#include "platform.h"
#include "platform_task.h"

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
 *	PE3 : encoder A1 left wheel
 *	PE4 : encoder B1
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
		},
		[1] = {
			.pin_id = PIN1_bp,
		},
		[2] = {
			.pin_id = PIN2_bp,
		},
		[3] = {
			.pin_id = PIN3_bp,
			.zone = AS_ZONE_REAR,
		},
		[4] = {
			.pin_id = PIN4_bp,
			.zone = AS_ZONE_FRONT,
		},
		[5] = {
			.pin_id = PIN5_bp,
			.zone = AS_ZONE_FRONT,
		},
		[6] = {
			.pin_id = PIN6_bp,
			.zone = AS_ZONE_FRONT,
		},
		[7] = {
			.pin_id = PIN7_bp,
			.zone = AS_ZONE_FRONT,
		},
	}
};
#endif /* CONFIG_ANALOG_SENSORS */

qdec_t encoders[] = {
	{
		/* left motor */
		.pin_port = &PORTE,
		.pin_qdph0 = PIN3_bp,
		.pin_qdph90 = PIN4_bp,
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

#if defined(CONFIG_SD21)
sd21_t sd21 = {
	.twi = &TWIC,
	.twi_speed_khz = 100,

	.servos_nb = 10,
	.servos = {
		[0] = {
			.value_init = 1200,
			.value_open = 1200,
			.value_close = 2400,
		},
		[9] = {
			.value_init = 2500,
			.value_open = 1800,
			.value_close = 700,
		},
	},
};
#endif /* CONFIG_SD21 */

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
			.direction_pin_id = PIN4_bp,
			.direction_inverse_polarity = TRUE,
			.pwm_channel = PIN0_bp,
			.offset = 100,
		},
		[HBRIDGE_MOTOR_RIGHT] = {
			/* right motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN5_bp,
			.direction_inverse_polarity = FALSE,
			.pwm_channel = PIN1_bp,
			.offset = 100,
		},
	},
};

controller_t controller = {
	.wheels_distance = WHEELS_DISTANCE,

	.linear_speed_pid = {
		.kp = 1.5,
		.ki = 0.2,
		.kd = 0,
	},
	.angular_speed_pid = {
		.kp = 1.5,
		.ki = 0.2,
		.kd = 0,
	},
	.linear_pose_pid = {
		.kp = 1,
		.ki = 0,
		.kd = 20,
	},
	.angular_pose_pid = {
		.kp = 1,
		.ki = 0,
		.kd = 20,
	},

	.min_distance_for_angular_switch = 500,
	.min_angle_for_pose_reached = 100,
};

/* This global object contains all numerical logs references (vectors, etc.) */
datalog_t datalog;

inline func_cb_t mach_get_ctrl_loop_pre_pfn()
{
	return NULL;
}

inline func_cb_t mach_get_ctrl_loop_post_pfn()
{
	return NULL;
}

inline func_cb_t mach_get_end_of_game_pfn()
{
	return NULL;
}

pose_t mach_trajectory_get_route_update(void)
{
	static uint8_t latest_pos_idx = 0;
	pose_t pos_list[2];

	pos_list[0].x = 4000.0; /*!< x-position [pulse] */
	pos_list[0].y = 0.0; /*!< y-position [pulse] */
	pos_list[0].O = 0.0; /*!< 0-orientation [pulse] */

	pos_list[1].x = 0.0; /*!< x-position [pulse] */
	pos_list[1].y = 0.0; /*!< y-position [pulse] */
	pos_list[1].O = 0.0; /*!< 0-orientation [pulse] */

	if (controller_is_pose_reached(&controller)) {
		latest_pos_idx ++;
		latest_pos_idx %= 2;
	}

	return pos_list[latest_pos_idx];
}

uint8_t mach_is_zone_obscured(analog_sensor_zone_t zone)
{
	uint8_t stop = 0;
	return stop;
}

uint8_t mach_is_game_launched(void)
{
	/* TODO: return GPIO of starter switch */
	return TRUE;
}

uint8_t mach_is_camp_yellow(void)
{
	/* TODO: return GPIO of color switch */
	return TRUE;
}

static void mach_pinmux_setup(void)
{
#if defined(__AVR__)
	/* analog to digital conversion */
	PORTA.DIR = 0x00; /*!< PORTA as input pin */
	PORTA.OUT = 0x00;
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;

	/* twi configuration pin */
	//PORTC.DIRSET = PIN0_bm; /*!< PC0 (SDA) as output pin */
	//PORTC.DIRSET = PIN1_bm; /*!< PC1 (SCL) as output pin */
	/* usart configuration pin */
	PORTC.DIRCLR = PIN2_bm; /*!< PC2 (RDX0) as input pin */
	PORTC.DIRSET = PIN3_bm; /*!< PC3 (TXD0) as output pin */

	/**
	 * PORTH and PORTJ as digital input
	 */
//	PORTH.DIR = 0x00;
//	PORTJ.DIRCLR = PIN0_bm;
//	PORTJ.DIRCLR = PIN1_bm;
#endif
}

void mach_sched_init()
{
	msched_init(10/*ms*/, &TCC0);
}

void mach_sched_run()
{
	kos_run();
}

console_t usartc0_console = {
	.usart = &USART_CONSOLE,
	.speed = 115200,
};

void mach_setup(void)
{
#if F_CPU == 32000000UL
	clksys_intrc_32MHz_setup();
#endif
	mach_pinmux_setup();

#ifdef CONFIG_ENABLE_LOGGING
	/* setup logs through usart communication */
	console_init(&usartc0_console);
#endif

#ifdef CONFIG_ANALOG_SENSORS
	/* setup analog conversion */
	analog_sensor_setup(&ana_sensors);
#endif

#if defined(CONFIG_SD21)
	/* setup TWI communication with SD21 */
	sd21_setup(&sd21);
#endif /* CONFIG_SD21 */

	//action_setup(); /* TODO: commenter pour debug */

	hbridge_setup(&hbridges);

	/* setup qdec */
	qdec_setup(&encoders[0]);
	qdec_setup(&encoders[1]);

	/* controller setup */
	odometry_setup(WHEELS_DISTANCE);

#if defined(__AVR__)
	/* Programmable Multilevel Interrupt Controller */
	PMIC.CTRL |= PMIC_LOLVLEN_bm; /* Low-level Interrupt Enable */

	/* global interrupt enable */
	sei();
#endif
	log_vect_init(&datalog, NULL, /*400,*/
			COL_INT16, "left_speed",
			COL_INT16, "right_speed",
			COL_INT16, "left_command",
			COL_INT16, "right_command",
			COL_DOUBLE, "robot_speed.distance",
			//COL_DOUBLE, "robot_speed.angle",
			COL_DOUBLE, "speed_order.distance",
			//COL_DOUBLE, "speed_order.angle",
			COL_END);
}
