#include "encoder.h"
#include "log.h"
#include "platform.h"
#include "qdec.h"

/**
 *
 */
polar_t encoder_read(void)
{
	polar_t robot_speed;

	int16_t left_speed = qdec_read(&encoders[0]);
	int16_t right_speed = qdec_read(&encoders[1]);

	print_dbg("encoders(L,R) = (%d, %d)\n", left_speed, right_speed);

	/* update speed */
	robot_speed.distance = (right_speed + left_speed) / 2.0;
	robot_speed.angle = right_speed - left_speed;

	print_dbg("(dist, angle) = (%d, %d)\n",
		  robot_speed.distance, robot_speed.angle);

	return robot_speed;
}
