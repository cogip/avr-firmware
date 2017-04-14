#ifndef PATH_H_
#define PATH_H_

#include <stdio.h>
#include "odometry.h"

/* POINT */
typedef struct point
{
	pose_t pose;
	int (*callback)(void *data);
	struct point *p_next;
	struct point *p_prev;
} actionpoint_t;

/* PATH*/
typedef struct path
{
	size_t length;
	actionpoint_t *p_tail;
	actionpoint_t *p_head;
	actionpoint_t *p_current;
} path_t;

path_t *path_new(void);
path_t *path_append(path_t *p_list, pose_t pose, int (*callback)(void *data));
path_t *path_prepend(path_t *p_list, pose_t pose, int (*callback)(void *data));
void path_display(path_t *path);

#endif /* PATH_H_ */
