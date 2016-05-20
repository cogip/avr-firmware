/**
 * \file odometry.h
 *
 * \date 2 avr. 2014
 * \author ldo
 */

#ifndef ODOMETRY_H_
#define ODOMETRY_H_

#include <math.h>
#include <avr/io.h>
#include <stdlib.h>

#define SEGMENT 0
#define ARC 1

/**
 * \struct pose
 */
typedef struct
{
  double x; /*!< x-position [pulse] */
  double y; /*!< y-position [pulse] */
  double O; /*!< 0-orientation [pulse] */
} pose;

/**
 * \struct polar_pose
 */
typedef struct
{
  double d; /*!< distance position [pulse] */
  double a; /*!< angle position [pulse] */
  double O; /*!< 0-orientation [pulse] */
} polar_pose; /* polar pose */

/**
 * \fn void odometry_setup (pose *p, double d)
 * \brief odometry pose and wheels_distance setup
 * \param p : robot pose
 * \param d : distance between wheels [pulse]
 * */
void
odometry_setup (double d);

void
pose_reset (pose *p);

/**
 * \fn void pose_update (pose *p, double x, double y, double O)
 * \brief update pose
 * \param p pose to update
 * \param x
 * \param y
 * \param O
 *  */
void
pose_update (pose *p, double x, double y, double O);

/**
 * \fn void odometry_update (const double distance, const double angle, const uint8_t approximation)
 * \brief update new robot pose (x, y, O) by ARC or SEGMENT approximation
 * \param distance : delta value for distance [pulse]
 * \param angle : delta value for angle [pulse]
 * \param approximation : SEGMENT (default) or ARC
 * */
void
odometry_update (pose *p, const double distance, const double angle,
		 const uint8_t approximation);

#endif /* ODOMETRY_H_ */
