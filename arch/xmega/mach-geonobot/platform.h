#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "qdec.h"
#include "hbridge.h"

#define HBRIDGE_MOTOR_LEFT		0
#define HBRIDGE_MOTOR_RIGHT		1

#define QDEC_LINE_COUNT			500

extern qdec_t encoders[];
extern hbridge_t hbridges;

#endif /* PLATFORM_H_ */
