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

#if defined(CONFIG_SD21)
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

/* Robot path */
path_t *path;

/* This global object contains all numerical logs references (vectors, etc.) */
datalog_t datalog;

inline func_cb_t mach_get_end_of_game_pfn()
{
	return NULL;
}

/* Return next action point */
pose_t mach_trajectory_get_route_update(void)
{
       pose_t empty = {0,};
       return empty;
       pose_t pose = {0, 0, 0};

       if ((path->p_current == NULL))
       {
               if (path->p_head != NULL)
               {
                       path->p_current = path->p_head;
                       pose = path->p_current->pose;
               }
       }
       else
       {
               path->p_current = path->p_current->p_next;
               pose = path->p_current->pose;
       }

       return pose;
}

uint8_t mach_stop_robot(void)
{
	uint8_t stop = 0;
	return stop;
}

void mach_path_setup(void)
{
       path = path_new();
       path = path_append(path, (pose_t){.x=1000,.y=0,.O=90}, NULL);
       path = path_prepend(path, (pose_t){.x=500,.y=0,.O=90}, NULL);
       path = path_append(path, (pose_t){.x=1000,.y=0,.O=180}, NULL);
       path_display(path);
}

static void mach_pinmux_setup(void)
{
#if defined(__AVR__)
	/* analog to digital conversion */
	PORTA.DIR = 0x00; /*!< PORTA as input pin */
	PORTA.OUT = 0x00;
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;

	/* twi configuration pin */
	PORTC.DIRSET = PIN1_bm; /*!< PC1 (SCL) as output pin */
	/* usart configuration pin */
	PORTC.DIRCLR = PIN2_bm; /*!< PC2 (RDX0) as input pin */
	PORTC.DIRSET = PIN3_bm; /*!< PC3 (TXD0) as output pin */
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

	mach_path_setup();

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
