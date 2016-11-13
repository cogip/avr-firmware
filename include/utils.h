/*
 * utils.h
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#ifndef UTILS_H_
#define UTILS_H_

typedef void (*func_cb_t)(void);

#define FALSE	(0)
#define TRUE	(!FALSE)

#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */
#endif

#define square(__x)	(__x * __x)

#endif /* UTILS_H_ */
