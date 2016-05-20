/*
 * sensor.h
 *
 *  Created on: 6 mai 2015
 *      Author: ldo
 */

#ifndef SENSOR_H_
#define SENSOR_H_

uint8_t
detect_right_cup (void);
uint8_t
detect_left_cup (void);

uint8_t
detect_elevator_up (void);
uint8_t
detect_elevator_down (void);
uint8_t
detect_spot (void);

uint8_t
detect_color (void);
uint8_t
detect_start (void);


uint8_t
stop_robot (uint8_t *ir_ids, uint8_t ir_nb);

#endif /* SENSOR_H_ */
