#include <stdio.h>

#include "utils.h"

inline double limit_angle_rad(double O)
{
	while (O > M_PI)
                O -= 2.0 * M_PI;

        while (O < -M_PI)
                O += 2.0 * M_PI;

	return O;
}
