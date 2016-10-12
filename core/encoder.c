#include <stdio.h>

#include "encoder.h"
#include "log.h"
#include "platform.h"
#include "qdec.h"

/**
 *
 */
int8_t display_dbg = FALSE;

#define print_dbg(...) \
do { \
	if (display_dbg) \
		printf(__VA_ARGS__); \
} while(0)

polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = qdec_read(&encoders[0]);
	int16_t right_speed = qdec_read(&encoders[1]);

	print_dbg("[%4d, %4d]\r", left_speed, right_speed);

	/* update speed */
	robot_speed.distance = (right_speed + left_speed) / 2.0;
	robot_speed.angle = right_speed - left_speed;

	//print_dbg("(dist, angle) = (%d, %d)\n",
	//	  robot_speed.distance, robot_speed.angle);

	return robot_speed;
}

#if defined(CONFIG_CALIBRATION)

#define CAL_USAGE \
"Calibration consists to move forward then backward (multiples times)\n" \
"the whole bot on a table, for a given known distance\n" \
"\n"\
"\t'S' to start/stop a calibration session\n"\
""

static void encoder_calibration_usage(void)
{
	printf("\n>>> Entering encoder calibration mode\n\n");

	printf("%s", CAL_USAGE);
	printf("\n");
	printf("\t'h' to display this help\n");
	printf("\t'q' to quit\n");
	printf("\n");
}

void encoder_enter_calibration(void)
{
	int c;
	uint8_t quit = 0;

	encoder_calibration_usage();

	while (!quit) {

		/* display prompt */
		printf("$ ");

		/* wait for command */
		c = mach_getchar_or_yield();
		printf("%c\n", c);

		switch (c) {
		case 'S':
			display_dbg = !display_dbg;
			printf("display_dbg = %s\n", display_dbg ? "True" : "False");
			break;
		case 'h':
			encoder_calibration_usage();
			break;
		case 'q':
			quit = 1;
			break;
		default:
			printf("\n");
			break;
		}
	}
}
#endif /* CONFIG_CALIBRATION */
