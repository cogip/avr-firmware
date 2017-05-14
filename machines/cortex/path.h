#ifndef PATH_H_

#include "action.h"
#include "odometry.h"

#define POSE_INITIAL			{ 873, 163, 45 *PULSE_PER_DEGREE }

typedef struct {
	pose_t pos;
	action_t act;
} path_pose_t;

extern path_pose_t path_game_yellow[];
extern uint8_t path_game_yellow_nb;

#endif /* PATH_H_ */
