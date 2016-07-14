#ifndef ENCODER_H_
#define ENCODER_H_

#include "odometry.h"

polar_t encoder_read(void);

#if defined(CONFIG_CALIBRATION)
void encoder_enter_calibration(void);
#endif

#endif /* ENCODER_H_ */
