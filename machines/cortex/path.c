#include <stdio.h>

#include "path.h"

#include "utils.h"

//#define CONFIG_PATH_HOMOLOGATION_2017
//#define CONFIG_PATH_TEST_ORING
#define CONFIG_PATH_2017

#if defined(CONFIG_PATH_HOMOLOGATION_2017)
static path_pose_t poses_homologation_yellow[] = {
	{ .pos = {.x =  873, .y =  163, .O =   45, }, .act = NULL, }, /* POSE_INITIAL */
	{ .pos = {.x = 1400, .y =  500, .O =  135, }, .act = NULL, },
	{ .pos = {.x = 1000, .y =  900, .O =  -90, }, .act = NULL, },
	{ .pos = {.x =  950, .y =  400, .O =  -90, }, .act = NULL, },
};

path_t path_homologation_yellow = {
	/* static cfg */
	.play_in_loop = FALSE,
	.nb_pose = 4,
	.poses = poses_homologation_yellow,
};

#elif defined(CONFIG_PATH_TEST_ORING)
static path_pose_t poses_homologation_yellow[] = {
	{ .pos = {.x =    0, .y =    0, .O =    0, }, .act = NULL, .allow_reverse = TRUE}, /* POSE_INITIAL */
	{ .pos = {.x = 1500, .y =    0, .O =    0, }, .act = NULL, .allow_reverse = TRUE},
	{ .pos = {.x = 1500, .y =    0, .O =   90, }, .act = NULL, .allow_reverse = TRUE},
	{ .pos = {.x = 1500, .y =  300, .O =  180, }, .act = NULL, .allow_reverse = TRUE},
};

path_t path_homologation_yellow = {
	/* static cfg */
	.play_in_loop = TRUE,
	.nb_pose = 4,
	.poses = poses_homologation_yellow,
};

#elif defined(CONFIG_PATH_2017)
static path_pose_t path_game_yellow[] = {
	{ .pos = {.x =  873 /* + 22*/ + 10 - 5, .y =  163-35+10, .O =   45, }, .act = NULL, }, /* POSE_INITIAL */

	/* Loading point high priority */
	{ .pos = {.x = 1117 /*+ 22*/, .y =  225, .O =    0, }, .act = act_open_front_right_arm, .collision_disable = TRUE },
	{ .pos = {.x = 1090 /*+ 22*/, .y =  225, .O =    0, }, .act = act_catch_module_front_right, .allow_reverse = TRUE, .collision_disable = TRUE },
	{ .pos = {.x = 1187 /*+ 22*/, .y =  235, .O =    0, }, .act = act_open_rear_right_arm, },
	{ .pos = {.x = 1219 /*+ 22*/, .y =  235, .O =    0, }, .act = act_catch_module_rear_right, },
	{ .pos = {.x = 1260 /*+ 22*/, .y =  235, .O =    0, }, .act = NULL, }, // degagement avant rotation
// right side only
//	{ .pos = {.x = 1187 /*+ 22*/, .y =  235,	.O =  180, }, .act = act_catch_module_front_left, },
//	{ .pos = {.x = 1117 /*+ 22*/, .y =  235, .O =  180, }, .act = act_catch_module_rear_left, },
// right side only

	/* Unloading point high priority */

	/* 1) right side */
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 900,	.O =   90, }, .act = act_open_front_right_arm /* open front right ventouse */, .allow_reverse = TRUE },
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 1300,	.O =   90, }, .act = act_stop_front_right_pump /* stop front right pump */, },
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 1200,	.O =   90, }, .act = act_close_FR_arm_open_RR_arm, .allow_reverse = TRUE }, /* note: FR module should drop */
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 1280,	.O =   90, }, .act = act_stop_rear_right_pump},
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 1300,	.O =   90, }, .act = act_close_rear_right_arm},
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 1000,	.O =   90, }, .act = act_open_front_right_arm /* open front right ventouse */, .allow_reverse = TRUE },
	{ .pos = {.x = 1240 /*+ 22*/ + 5, .y = 1300,	.O =   90, }, .act = act_close_front_right_arm /* close front right ventouse */, .allow_reverse = TRUE },

	{ .pos = {.x = 1000 /*+ 22*/ + 5, .y =  1000, .O =   90, }, .act = NULL, .allow_reverse = TRUE },
	{ .pos = {.x = 1000 /*+ 22*/ + 5, .y =  400, .O =   90, }, .act = NULL, .allow_reverse = TRUE },

	/* pichenette */
	/*{ .pos = {.x = 1060 + 22, .y = 1000,	.O =   90, }, .act = act_open_front_left_arm, .allow_reverse = TRUE },
	{ .pos = {.x = 1060 + 22, .y =  1180, .O =   90, }, .act = NULL, .allow_reverse = TRUE },
	{ .pos = {.x = 1060 + 22, .y = 1180,	.O =   45, }, .act = act_close_front_left_arm, .allow_reverse = TRUE },
	{ .pos = {.x = 1060 + 22, .y = 1180,	.O =   90, }, .act = NULL, .allow_reverse = TRUE },*/

	/* module 2 */
	/*{ .pos = {.x = 800 + 22, .y = 1100,	.O =   0, }, .act = NULL, .allow_reverse = TRUE },
	{ .pos = {.x = 550, + 22 .y = 1100,	.O =   180, }, .act = NULL, .allow_reverse = TRUE },
	{ .pos = {.x = 1000 + 22, .y = 1000,	.O =   90, }, .act = NULL, .allow_reverse = TRUE },
	{ .pos = {.x = 1000 + 22, .y =  400, .O =   90, }, .act = NULL, .allow_reverse = TRUE },*/

	/* aspiration */
	{ .pos = {.x = 950 /*+ 22*/, .y =  540, .O =   180, }, .act = NULL},
	{ .pos = {.x = 885 /*+ 22*/, .y =  540, .O =   180, }, .act = start_turbine}, // aspiration
	{ .pos = {.x = 950 /*+ 22*/, .y =  540, .O =   180, }, .act = NULL, .allow_reverse = TRUE },
	{ .pos = {.x = 1000 /*+ 22*/, .y =  350, .O =   -135, }, .act = stop_turbine }, // desaspiration
	{ .pos = {.x = 1200 /*+ 22*/, .y =  500, .O =   -135, }, .act = NULL, .allow_reverse = TRUE }, // desaspiration
};

path_t path_yellow = {
	/* static cfg */
	.play_in_loop = FALSE,
	.nb_pose = 20,
	.poses = path_game_yellow,
};
#endif
