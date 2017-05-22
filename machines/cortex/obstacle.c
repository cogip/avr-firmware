#include <stdio.h>
#include <math.h>

#include "avoidance.h"
#include "obstacle.h"
#include "platform.h"
#include "trigonometry.h"

/* Init all known fixed obstacles on map */
void mach_fixed_obstacles_init(void)
{
	polygon_t polygon;
	uint8_t nb_vertices;

	polygon.count = 0;
	nb_vertices = 4;
	if (nb_vertices < POLY_MAX_POINTS)
	{
		polygon.points[polygon.count++] = (pose_t){.x = 1000, .y = 600};
		polygon.points[polygon.count++] = (pose_t){.x = 1400, .y = 600};
		polygon.points[polygon.count++] = (pose_t){.x = 1400, .y = 1000};
		polygon.points[polygon.count++] = (pose_t){.x = 1000, .y = 1000};
		add_polygon(&polygon);
	}
}

/* Init all known fixed obstacles on map */
void add_dyn_obstacle(const pose_t *robot_pose, analog_sensor_zone_t zone)
{
	polygon_t polygon;
	uint8_t nb_vertices;
	double direction = 0;

	pose_t robot_pose_tmp = *robot_pose;
	robot_pose_tmp.O *= PULSE_PER_DEGREE;
	robot_pose_tmp.O = DEG2RAD(robot_pose_tmp.O);

	polygon.count = 0;
	nb_vertices = 4;
	if (nb_vertices < POLY_MAX_POINTS)
	{

		if (zone == AS_ZONE_REAR)
		{
			direction = 1;
		}

		/* Right reference*/
		pose_t ref_pos_right = (pose_t){.x = robot_pose_tmp.x + OBSTACLE_DYN_LENGTH * cos(robot_pose_tmp.O - M_PI/2),
							 .y = robot_pose_tmp.y + OBSTACLE_DYN_LENGTH * sin(robot_pose_tmp.O - M_PI/2),
							 .O = robot_pose_tmp.O + direction * M_PI};
		/* Left reference */
		pose_t ref_pos_left = (pose_t){.x = robot_pose_tmp.x + OBSTACLE_DYN_LENGTH * cos(robot_pose_tmp.O + M_PI/2),
							 .y = robot_pose_tmp.y + OBSTACLE_DYN_LENGTH * sin(robot_pose_tmp.O + M_PI/2),
							 .O = robot_pose_tmp.O + direction * M_PI};

		/* Right points */
		polygon.points[polygon.count] = (pose_t){.x = ref_pos_right.x + OBSTACLE_DYN_LENGTH * cos(ref_pos_right.O),
							   .y = ref_pos_right.y + OBSTACLE_DYN_LENGTH * sin(ref_pos_right.O)};
		polygon.count++;
		polygon.points[polygon.count] = (pose_t){.x = ref_pos_right.x + (OBSTACLE_DYN_LENGTH + OBSTACLE_DYN_SIZE) * cos(ref_pos_right.O),
							 .y = ref_pos_right.y + (OBSTACLE_DYN_LENGTH + OBSTACLE_DYN_SIZE) * sin(ref_pos_right.O)};
		polygon.count++;

		/* Left points */
		polygon.points[polygon.count] = (pose_t){.x = ref_pos_left.x + (OBSTACLE_DYN_LENGTH + OBSTACLE_DYN_SIZE) * cos(ref_pos_left.O),
							 .y = ref_pos_left.y + (OBSTACLE_DYN_LENGTH + OBSTACLE_DYN_SIZE) * sin(ref_pos_left.O)};
		polygon.count++;
		polygon.points[polygon.count] = (pose_t){.x = ref_pos_left.x + OBSTACLE_DYN_LENGTH * cos(ref_pos_left.O),
							 .y = ref_pos_left.y + OBSTACLE_DYN_LENGTH * sin(ref_pos_left.O)};
		polygon.count++;
		add_dyn_polygon(&polygon);
	}
}
