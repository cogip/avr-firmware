#ifndef PATH_H_
#define PATH_H_

#include "action.h"
#include "odometry.h"

#define POSE_INITIAL			{ 873, 163, 45 }

typedef struct {
	pose_t pos;
	action_t act;
} path_pose_t;

extern path_pose_t path_game_yellow[];
extern uint8_t path_game_yellow_nb;

#endif /* PATH_H_ */
