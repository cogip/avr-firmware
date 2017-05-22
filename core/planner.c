#include "planner.h"

#include "console.h"
#include "controller.h"
#include "kos.h"
#include "obstacle.h"
#include "platform.h"

static uint16_t game_time;
static uint8_t game_started = FALSE;

/* periodic task */
/* sched period = 20ms -> ticks freq is 1/0.02 = 50 Hz */
#define TASK_PERIOD_MS		(20 * 5)

#define TASK_FREQ_HZ		(1000 / TASK_PERIOD_MS)
#define GAME_DURATION_SEC	90
#define GAME_DURATION_TICKS	(GAME_DURATION_SEC * TASK_FREQ_HZ)

static void show_game_time()
{
	static uint8_t _secs = TASK_FREQ_HZ;

	if (! --_secs) {
		_secs = TASK_FREQ_HZ;
		print_info ("Game time = %d\n",
			    game_time / TASK_FREQ_HZ);
	}
}

void planner_start_game(void)
{
	/* TODO: send pose_initial, pose_order & speed_order to controller */
	controller_set_mode(&controller, CTRL_STATE_INGAME);
	game_started = TRUE;
}

void task_planner(void)
{
	uint8_t stop = FALSE;
	func_cb_t pfn_evtloop_end_of_game = mach_get_end_of_game_pfn();
	pose_t	pose_order		= { 0, 0, 0 };
	polar_t	speed_order		= { 0, 0 };
	pose_t robot_pose		= POSE_INITIAL;

	print_info("Game planner started\n");

	for (;;)
	{
		if (!game_started)
			goto yield_point;

		kos_set_next_schedule_delay_ms(TASK_PERIOD_MS);

		if (pfn_evtloop_end_of_game && game_time >= GAME_DURATION_TICKS)
			(*pfn_evtloop_end_of_game)();

		/* while starter switch is not release we wait */
		if (!mach_is_game_launched())
			goto yield_point;

		if (game_time >= GAME_DURATION_TICKS) {
			cons_printf(">>>>\n");
			controller_set_mode(&controller, CTRL_STATE_STOP);
			break;
		}

		if (!game_time) {
			cons_printf("<<<< polar_simu.csv\n");
			cons_printf("pose_order_x,pose_order_y,pose_order_a,"
				    "pose_current_x,pose_current_y,pose_current_a,"
				    "position_error_l,position_error_a,"
				    "speed_order_l,speed_order_a,"
				    "speed_current_l,speed_current_a,"
				    "game_time,"
				    "\n");
		}

		game_time++;
		show_game_time();

		/* collision detection */
		robot_pose = get_robot_pose();
		stop = mach_stop_robot(&robot_pose);

		pose_order = mach_trajectory_get_route_update(&robot_pose, stop);

		controller_set_pose_to_reach(&controller, pose_order);

		/* speed order in position = 60 pulses / 20ms */
		speed_order.distance = 150;
		/* speed order in angle? = 60 pulses / 20ms */
		speed_order.angle = 150 / 2;
		controller_set_speed_order(&controller, speed_order);

yield_point:
		kos_yield();
	}

//	controller.mode = CTRL_STATE_INGAME;
//	cons_printf("calibration ended\n");
	kos_task_exit();
}

