#ifndef ACTION_H_

//typedef struct {
//	pose_t pos;
//	action_t action;
//} action_t;

#include "utils.h"

typedef func_cb_t action_t;

/* catch lunar modules */
void act_catch_module_front_right(void);
void act_catch_module_rear_right(void);
void act_catch_module_front_left(void);
void act_catch_module_rear_left(void);

/* Unload "central village" sequence (right side) */
void act_open_front_right_arm(void);
void act_open_front_left_arm(void);
void act_close_front_right_arm(void);
void act_close_front_left_arm(void);
void act_open_rear_right_arm(void);
void act_stop_front_right_pump(void);
void act_close_FR_arm_open_RR_arm(void);
void act_stop_rear_right_pump(void);
void act_close_rear_right_arm(void);

//void act_unload_front_right_module(void);

#endif /* ACTION_H_ */
