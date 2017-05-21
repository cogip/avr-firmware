#include <stdio.h>

#include "path.h"

#include "utils.h"

// TODO: do not use Rear gear.
static path_pose_t poses_homologation_yellow[] = {
	{ /* POSE_INITIAL */
		.pos = {
			.x = 873,
			.y = 163,
			.O = 45,
		},
	},
	{
		.pos = {
			.x = 1400,
			.y = 500,
			.O = 135,
		},
		.act = NULL,
	},
	{
		.pos = {
			.x = 1000,
			.y = 900,
			.O = -90,
		},
		.act = NULL,
	},
	{
		.pos = {
			.x = 950,
			.y = 400,
			.O = -90,
		},
		.act = NULL,
	},
};

path_t path_homologation_yellow = {
	/* static cfg */
	.play_in_loop = FALSE,
	.nb_pose = 4,
	.poses = poses_homologation_yellow,
};

#if 0
path_pose_t path_game_yellow[] = {
	[0] = {
		.pos = {
			.x = 1117,
			.y = 260,
			.O = 0,
		},
		.act = NULL,
	},
	[1] = {
		.pos = {
			.x = 1187,
			.y = 260,
			.O = 180,
		},
		.act = NULL,
	},
	[2] = {
		.pos = {
			.x = 1187,
			.y = 260,
			.O = 180,
		},
		.act = NULL,
	},
	[3] = {
		.pos = {
			.x = 1117,
			.y = 260,
			.O = 179,
		},
		.act = NULL,
	},
	[4] = {
		.pos = {
			.x = 1267,
			.y = 1150,
			.O = 90,
		},
		.act = NULL,
	},
	[5] = {
		.pos = {
			.x = 1267,
			.y = 1120,
			.O = 90,
		},
		.act = NULL,
	},
};

uint8_t path_game_yellow_nb = 6;
#endif
