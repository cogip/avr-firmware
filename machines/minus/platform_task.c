#include <stdio.h>
#include <stdlib.h>

#include "console.h"
#include "controller.h"
#include "encoder.h"
#include "hbridge.h"
#include "kos.h"
#include "log.h"
#include "mcurses.h"
#include "platform.h"
#include "platform_task.h"
#include "usart.h"
#include "qdec.h"

#if defined(CONFIG_CALIBRATION)
static void mach_calibration_usage(void)
{
	cons_printf("\n>>> Entering calibration mode\n\n");

	cons_printf("\t'o' to calibrate odometry\n");
	cons_printf("\t'p' to calibrate hbridge & PWM ctrl\n");
#if defined(CONFIG_SD21)
	printf("\t's' to calibrate servos (sd21 card)\n");
#endif
	cons_printf("\t'r' to calibrate controller\n");
	cons_printf("\n");
	cons_printf("\t'h' to display this help\n");
	cons_printf("\t'e' to exit calibration mode\n");
	cons_printf("\n");
}

static void mach_enter_calibration_mode(void)
{
	int16_t autoboot_ms = 3000;
	int c;
	uint8_t quit = 0;

	/* wait for keypress, or schedule */
	while (!usart_is_data_arrived(&USART_CONSOLE)) {

		cons_printf("Press a key to enter calibration... %ds remaining\r",
			autoboot_ms / 1000);

		kos_set_next_schedule_delay_ms(250);
		autoboot_ms -= 250;

		if (autoboot_ms > 0)
			kos_yield();
		else
			/* time elapsed, we bypass calibration
			 * and continue to game mode. */
			goto exit_point;
	}
	cons_printf("\n\n");
	getchar();

	mcurses_init();
	mach_calibration_usage();

	while (!quit) {

		/* display prompt */
		cons_printf("$ ");

		/* wait for command */
		c = cons_getchar();
		cons_printf("%c\n", c);

		switch (c) {
		case 'o':
			encoder_enter_calibration();
			/* TODO; odometry_enter_calibration */
			break;
		case 'p':
			hbridge_enter_calibration(&hbridges);
			break;
#if defined(CONFIG_SD21)
		case 's':
			sd21_enter_calibration(&sd21);
			break;
#endif
		case 'r':
			controller_enter_calibration();
			break;
		case 'h':
			mach_calibration_usage();
			break;
		case 'e':
			quit = 1;
			break;
		default:
			cons_printf("\n");
			break;
		}
	}

exit_point:
	controller.mode = CTRL_STATE_INGAME;
	cons_printf("calibration ended\n");
	kos_task_exit();
}
#endif /* CONFIG_CALIBRATION */

/*
 * Tasks registration
 */

#define TASK_CALIB_STACK	256
#define TASK_CTRL_STACK		256

void mach_tasks_init()
{
	kos_init();

#if defined(CONFIG_CALIBRATION)
	kos_new_task(mach_enter_calibration_mode, "CALIB", TASK_CALIB_STACK);
#endif
	kos_new_task(task_controller_update, "CTRL", TASK_CTRL_STACK);
}
