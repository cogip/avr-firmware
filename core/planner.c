#include "planner.h"

#include "avoidance.h"
#include "console.h"
#include "kos.h"
#include "platform.h"
#include "trigonometry.h"

/* Object variables (singleton) */
static uint16_t game_time = 0;
static uint8_t game_started = FALSE;
static path_t * path_yellow = NULL;

/* periodic task */
/* sched period = 20ms -> ticks freq is 1/0.02 = 50 Hz */
#define TASK_PERIOD_MS		(20 * 5)

#define TASK_FREQ_HZ		(1000 / TASK_PERIOD_MS)
#define GAME_DURATION_SEC	90
#define GAME_DURATION_TICKS	(GAME_DURATION_SEC * TASK_FREQ_HZ)

static void pose_yellow_to_blue(pose_t *pose)
{
	pose->x = 3000 - pose->x;
	pose->y = pose->y;
	pose->O = limit_angle_deg(180 - pose->O);
}

static inline void increment_current_pose_idx()
{
	if (path_yellow->current_pose_idx < path_yellow->nb_pose - 1)
		path_yellow->current_pose_idx += 1;
	else if (path_yellow->play_in_loop)
		path_yellow->current_pose_idx = 0;
}

static void show_game_time()
{
	static uint8_t _secs = TASK_FREQ_HZ;

	if (! --_secs) {
		_secs = TASK_FREQ_HZ;
		print_info ("Game time = %d\n",
			    game_time / TASK_FREQ_HZ);
	}
}

pose_t planner_get_path_pose_initial()
{
	path_t *path = path_yellow;
	pose_t pose = { 0, 0, 0 };

	if (path && path->poses && path->nb_pose)
		pose = path->poses[0].pos;

	return pose;
}

void planner_start_game(void)
{
	/* TODO: send pose_initial, pose_order & speed_order to controller */
	controller_set_mode(&controller, CTRL_STATE_INGAME);
	game_started = TRUE;
}

/* NOTE: TO INTEGRATE & REMOVE */
#if 0
static pose_t mach_trajectory_get_route_update(void)
{
	static pose_t pose_reached = POSE_INITIAL;
	static pose_t pose_to_reach;
	static int8_t latest_pos_idx = -1;
	static uint8_t index = 1;

	if (latest_pos_idx == -1)
	{
		latest_pos_idx = 0;
		pose_to_reach = path_game_yellow[latest_pos_idx].pos;
		set_start_finish(&pose_reached, &pose_to_reach);
		update_graph();
	}

	if (controller_is_pose_reached(&controller))
	{
		if ((pose_to_reach.x == path_game_yellow[latest_pos_idx].pos.x)
			&& (pose_to_reach.y == path_game_yellow[latest_pos_idx].pos.y))
		{
			if (latest_pos_idx + 1 < path_game_yellow_nb)
			{
				latest_pos_idx++;
			}
			set_start_finish(&pose_reached, &(path_game_yellow[latest_pos_idx].pos));
			update_graph();
			index = 1;
		}
		else
		{
			index++;
		}
		pose_reached = pose_to_reach;
	}

	pose_to_reach = avoidance(index);
	if ((pose_to_reach.x == path_game_yellow[latest_pos_idx].pos.x)
		&& (pose_to_reach.y == path_game_yellow[latest_pos_idx].pos.y))
	{
		controller_set_pose_intermediate(&controller, FALSE);
	}
	else
	{
		controller_set_pose_intermediate(&controller, TRUE);
	}

	return pose_to_reach;
}
#endif
/* NOTE: TO INTEGRATE & REMOVE */

void task_planner(void)
{
	analog_sensor_zone_t zone;
	func_cb_t pfn_evtloop_end_of_game = mach_get_end_of_game_pfn();
	pose_t	pose_order		= { 0, 0, 0 };
	polar_t	speed_order		= { 0, 0 };
	const uint8_t camp_yellow	= mach_is_camp_yellow();

	print_info("Game planner started\n");
	print_info("%s camp\n", camp_yellow ? "YELLOW" : "BLUE");

	path_yellow = mach_get_path_yellow();
	if (!path_yellow) {
		print_err("machine has no path\n");
		kos_task_exit();
	}

	/* mirror the points in place if selected camp is blue */
	if (!camp_yellow) {
		path_yellow->current_pose_idx = path_yellow->nb_pose;
		do {
			path_yellow->current_pose_idx -= 1;
			pose_yellow_to_blue(&path_yellow->poses[path_yellow->current_pose_idx].pos);
		}
		while (path_yellow->current_pose_idx);
	}

	/* object context initialisation */
	path_yellow->current_pose_idx = 0;

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


		/* ===== speed ===== */

		/* collision detection */
		if (controller_is_in_reverse(&controller))
			zone = AS_ZONE_REAR;
		else
			zone = AS_ZONE_FRONT;

		if (mach_is_zone_obscured(zone)) {
			speed_order.distance = 0;
		} else {
			/* max speed order in pulse_linear per ctrl period (20ms) */
			speed_order.distance = 150;
		}
		/* max speed order in pulse_angular per ctrl period (20ms) */
		speed_order.angle = 150 / 2;

		controller_set_speed_order(&controller, speed_order);

		/* ===== position ===== */
		if (controller_is_pose_reached(&controller)) {
			increment_current_pose_idx();
		}

		pose_order = path_yellow->poses[path_yellow->current_pose_idx].pos;

		controller_set_pose_to_reach(&controller, pose_order);

yield_point:
		kos_yield();
	}

//	controller.mode = CTRL_STATE_INGAME;
//	cons_printf("calibration ended\n");
	kos_task_exit();
}

