#include <stdio.h>
#include <stdlib.h>

#include "action.h"
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
#include "sensor.h"

static uint16_t tempo;

void mach_evtloop_end_of_game(void)
{
	open_pince();
	open_door();
	set_release_right_cup();
	set_release_left_cup();
}

void task_active_event_loop()
{
	while (controller.mode != CTRL_STATE_INGAME)
		kos_yield();

	printf("task_active_event_loop()\n");
	for (;;) {

		if (detect_start()) {
			gestion_tour();
		} else {
			attraper_cup();
			gestion_tour();
		}

		kos_yield();
	}
}

#if defined(CONFIG_CALIBRATION)
static void mach_calibration_usage(void)
{
	printf("\n>>> Entering calibration mode\n\n");

	printf("\t'p' to calibrate hbridge & PWM ctrl\n");
#if defined(CONFIG_SD21)
	printf("\t's' to calibrate servos (sd21 card)\n");
#endif
	printf("\t'r' to calibrate controller\n");
	printf("\n");
	printf("\t'h' to display this help\n");
	printf("\t'e' to exit calibration mode\n");
	printf("\n");
}

static void mach_enter_calibration_mode(void)
{
	int16_t autoboot_ms = 3000;
	int c;
	uint8_t quit = 0;

	/* wait for keypress, or schedule */
	while (!usart_is_data_arrived(&USART_CONSOLE)) {

		printf("Press a key to enter calibration... %ds remaining\r",
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
	printf("\n\n");
	getchar();

	mcurses_init();
	mach_calibration_usage();

	while (!quit) {

		/* display prompt */
		printf("$ ");

		/* wait for command */
		c = mach_getchar_or_yield();
		printf("%c\n", c);

		switch (c) {
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
			printf("\n");
			break;
		}
	}

exit_point:
	controller.mode = CTRL_STATE_INGAME;
	printf("calibration ended\n");
	kos_task_exit();
}
#endif /* CONFIG_CALIBRATION */

/*
 * Tasks registration
 */

#define TASK_CALIB_STACK	184
#define TASK_EVENT_STACK	128
#define TASK_CTRL_STACK		256

void mach_tasks_init()
{
	kos_init();

#if defined(CONFIG_CALIBRATION)
	kos_new_task(mach_enter_calibration_mode, "CALIB", TASK_CALIB_STACK);
#endif
	kos_new_task(task_active_event_loop, "EVNT", TASK_EVENT_STACK);
	kos_new_task(task_controller_update, "CTRL", TASK_CTRL_STACK);
}
