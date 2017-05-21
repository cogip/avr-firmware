#ifndef PLANNER_H_
#define PLANNER_H_

#include "odometry.h"
#include "utils.h"

typedef struct {
	pose_t pos;
	func_cb_t act;
} path_pose_t;

typedef struct {
	/* static cfg */
	uint8_t play_in_loop; /* for unit tests */
	uint8_t nb_pose;
	path_pose_t *poses;

	/* dynamic variables */
	uint8_t current_pose_idx;
} path_t;

pose_t planner_get_path_pose_initial();
void planner_start_game(void);

void task_planner(void);

#endif /* PLANNER_H_ */
