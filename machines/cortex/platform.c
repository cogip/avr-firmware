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
 *	PC0 (SDA) : I2C RGB Sensor Left + SD21
 *	PC1 (SCL) : I2C RGB Sensor Left + SD21
 *	PC2 (RX) : UART Debug communication
 *	PC3 (TX) : UART Debug communication
 *	PC4 (GPIO) : RGB LED Enable Left
 *	PC7 (GPIO) : RGB LED Enable Right
 * PORTD : Communication + PWM
 *	PD0 (SDA) : I2C RGB Sensor right
 *	PD1 (SCL) : I2C RGB Sensor right
 *	PD2 (OC0A) : PWM right motor
 *	PD3 (OCBA) : PWM left motor
 *	PD4 : DIR right motor
 *	PD5 : DIR left motor
 * PORTE : Timer + PWM
 *	PE4 : encoder A1 left wheel
 *	PE5 : encoder B1
 * PORTF : Timer decoder quadrature
 *	PF0 : encoder A2 right wheel
 *	PF1 : encoder B2
 *
 * use TCC0 as general timer
 * use TCE0 timer to generate PWM signal
 * use TCE1, TCF0 and TCF1 timers to decode quadrature
 */

#ifdef CONFIG_ANALOG_SENSORS
analog_sensors_t ana_sensors = {
	.adc = &ADCA,

	.sensors_nb = 6,
	.sensors = {
		/* Rear right: [10...80] cm - GP2Y0A21 - cal done */
		[0] = {
			.pin_id = PIN0_bp,

			.coeff_volts = 0.022,
			.const_volts = 0.010,
			.const_dist = -5.0,
			.dist_cm_max = 50,

			.zone = (AS_ZONE_REAR | AS_ZONE_RIGHT),
		},
		/* Front left: [10...80] cm - GP2Y0A21 - cal done */
		[1] = {
			.pin_id = PIN1_bp,

			.coeff_volts = 0.03,
			.const_volts = 0.009,
			.const_dist = 1.0,
			.dist_cm_max = 80,

			.zone = (AS_ZONE_FRONT | AS_ZONE_LEFT),
		},
		/* Front right: [10...60] cm - GP2D12 - cal done*/
		[2] = {
			.pin_id = PIN2_bp,

			.coeff_volts = 0.045,
			.const_volts = 0.027,
			.const_dist = 5.0,
			.dist_cm_max = 50,

			.zone = (AS_ZONE_FRONT | AS_ZONE_RIGHT),
		},
		/* Side left: [4...30] cm - GP2YD120X - cal done */
		[3] = {
			.pin_id = PIN5_bp,

			.coeff_volts = 0.052,
			.const_volts = 0.007,
			.const_dist = 0,
			.dist_cm_max = 40,

			.zone = AS_ZONE_LEFT,
		},
		/* Side right: [4...30] cm - GP2YD120X - cal done */
		[4] = {
			.pin_id = PIN6_bp,

			.coeff_volts = 0.052,
			.const_volts = 0.007,
			.const_dist = 0,
			.dist_cm_max = 40,

			.zone = AS_ZONE_RIGHT,
		},
		/* Rear left: [10...80] cm - GP2Y0A21 - cal done  */
		[5] = {
			.pin_id = PIN7_bp,

			.coeff_volts = 0.03,
			.const_volts = 0.009,
			.const_dist = 1.0,
			.dist_cm_max = 80,

			.zone = (AS_ZONE_REAR | AS_ZONE_LEFT),
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
		.polarity = 1,
	},
};

#if defined(CONFIG_SD21)
sd21_t sd21 = {
	.twi = &TWIC,
	.twi_speed_khz = 100,

	.servos_nb = 4,
	.servos = {
		/* Front-Left */
		[0] = {
			.value_init = 550/*1500*/,
			.value_open = 1850,
			.value_close = 550,
		},

		/* Bottom-Left */
		[1] = {
			.value_init = 1050/*1500*/,
			.value_open = 2100,
			.value_close = 1050,
		},

		/* Bottom-Right */
		[2] = {
			.value_init = 600/*1500*/,
			.value_open = 1875,
			.value_close = 600,
		},

		/* Front-Right */
		[3] = {
			.value_init = 2550/*1500*/,
			.value_open = 1150,
			.value_close = 2550,
		},

		///* Right arm */
		//[4] = {
		//	.value_init = 2400,
		//	.value_open = 1200,
		//	.value_close = 2400,
		//},
		///* Left arm */
		//[5] = {
		//	.value_init = 600,
		//	.value_open = 1800,
		//	.value_close = 600,
		//},
		///* Glasses right arm */
		//[6] = {
		//	.value_init = 600,
		//	.value_open = 600,
		//	.value_close = 2450,
		//},
		///* Glasses left arm */
		//[7] = {
		//	.value_init = 2600,
		//	.value_open = 2600,
		//	.value_close = 800,
		//},

		///* Clamp */
		//[8] = {
		//	.value_init = 1700,
		//	.value_open = 2120,
		//	.value_close = 1700,
		//},
	},
};
#endif /* CONFIG_SD21 */

/* TCE0 ClkIn == ClkPer / 8 == 4000 KHz */
/* Counter set to 200 for 20KHz output */
#define TC_MOTOR_PRESCALER		TC_CLKSEL_DIV8_gc
#define TC_MOTOR_PER_VALUE		200

hbridge_t hbridges = {
	.tc = &TCD0,
	.period = TC_MOTOR_PER_VALUE,
	.prescaler = TC_MOTOR_PRESCALER,

	.pwm_port = &PORTD, /* TODO: can be 'guessed' from timer ref above */

	.engine_nb = 2,
	.engines = {
		[HBRIDGE_MOTOR_LEFT] = {
			/* left motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN5_bp,
			.direction_inverse_polarity = FALSE,
			.pwm_channel = PIN3_bp,
			.offset = 0,
		},
		[HBRIDGE_MOTOR_RIGHT] = {
			/* right motor */
			.direction_pin_port = &PORTD,
			.direction_pin_id = PIN4_bp,
			.direction_inverse_polarity = TRUE,
			.pwm_channel = PIN2_bp,
			.offset = 0,
		},
	},
};

controller_t controller = {
	.wheels_distance = WHEELS_DISTANCE,

#if defined(__AVR__)
	.linear_speed_pid = {
		.kp = 2.0,
		.ki = 0.1,
		.kd = 0,
	},
	.angular_speed_pid = {
		.kp = 2.0,
		.ki = 0.1,
		.kd = 0,
	},
	.linear_pose_pid = {
		.kp = 0.050,
		.ki = 0,
		.kd = 0.4,
	},
	.angular_pose_pid = {
		.kp = 0.050,
		.ki = 0,
		.kd = 0.4,
	},
#else
	.linear_speed_pid = {
		.kp = 2.9,
		.ki = 0.15,
		.kd = 0,
	},
	.angular_speed_pid = {
		.kp = 2,
		.ki = 0.05,
		.kd = 0,
	},
	.linear_pose_pid = {
		.kp = 0.050,
		.ki = 0,
		.kd = 0.4,
	},
	.angular_pose_pid = {
		.kp = 0.050,
		.ki = 0,
		.kd = 0.4,
	},
#endif

	//.min_distance_for_angular_switch = 500,
	.min_distance_for_angular_switch = 100,
	.min_angle_for_pose_reached = 100,
};

/* This global object contains all numerical logs references (vectors, etc.) */
datalog_t datalog;

static void mach_post_ctrl_loop_func()
{
	analog_sensor_refresh_all(&ana_sensors);
}

inline func_cb_t mach_get_ctrl_loop_pre_pfn()
{
	return NULL;
}

inline func_cb_t mach_get_ctrl_loop_post_pfn()
{
	return mach_post_ctrl_loop_func;
}

inline func_cb_t mach_get_end_of_game_pfn()
{
	return NULL;
}

pose_t mach_trajectory_get_route_update(void)
{
	pose_t pose_to_reach;
	static uint8_t latest_pos_idx = 0;

	if (controller_get_pose_reached(&controller)
	    && latest_pos_idx + 1 < path_game_yellow_nb) {
		latest_pos_idx ++;
	}

	pose_to_reach = path_game_yellow[latest_pos_idx].pos;

	return pose_to_reach;
}

uint8_t mach_stop_robot(void)
{
	uint8_t stop = 0;
	return stop;
//	return analog_sensor_detect_obstacle (&ana_sensors, AS_ZONE_FRONT|AS_ZONE_REAR);
}


static void mach_pinmux_setup(void)
{
#if defined(__AVR__)
	/* analog to digital conversion */
	PORTA.DIR = 0x00; /*!< PORTA as input pin */
	PORTA.OUT = 0x00;
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;

	/* Port B - Jtag disable (fuse bit required) */
	MCU_MCUCR = MCU_JTAGD_bm; /* Fuse4 bit0 to set to 1 with flasher */

	/* twi configuration pin */
	PORTC.DIRSET = PIN1_bm; /*!< PC1 (SCL) as output pin */
	/* usart configuration pin */
	PORTC.DIRCLR = PIN2_bm; /*!< PC2 (RDX0) as input pin */
	PORTC.DIRSET = PIN3_bm; /*!< PC3 (TXD0) as output pin */
#endif

	/* Pumps, outputs all off. */
	//PORTB.DIR = 0xff; /*!< PORTB as output pin */
	//PORTB.DIRSET = 0xff;
	//PORTB.OUT = 0;

	gpio_set_direction(&PORTB, 0 /* pin_id */, TRUE);
	gpio_set_direction(&PORTB, 1 /* pin_id */, TRUE);
	gpio_set_direction(&PORTB, 2 /* pin_id */, TRUE);
	gpio_set_direction(&PORTB, 3 /* pin_id */, TRUE);
	gpio_set_direction(&PORTB, 4 /* pin_id */, TRUE);
	gpio_set_direction(&PORTB, 5 /* pin_id */, TRUE);

	gpio_set_output(&PORTB, PIN0_bp, 0);
	gpio_set_output(&PORTB, PIN1_bp, 0);
	gpio_set_output(&PORTB, PIN2_bp, 0);
	gpio_set_output(&PORTB, PIN3_bp, 0);
	gpio_set_output(&PORTB, PIN4_bp, 0);
	gpio_set_output(&PORTB, PIN5_bp, 0);
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
