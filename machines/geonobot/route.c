#include <stdio.h>

#include "action.h"
#include "controller.h"
#include "platform.h"
#include "route.h"
#include "sensor.h"

#define NB_POSE		21
// 10610 pour 1 m - 4650 pour 1/4 tour 90°

action_t route[NB_POSE] =
  {
    {
      { 2500, 0, 0 }, NULL, 0 },
    {
      { 500, 0, 0 }, NULL, 0 },
    {
      { 820, 290, 5 }, NULL, 1 },
    {
      { 1185, 370, 110 }, NULL, 0 },
    {
      { 1115, 670, 0 }, NULL, 0 },
    {
      { 1250, 670, 90 }, NULL, 0 },
    {
      { 1250, 700, 90 }, set_flag_tower_down, 0 },
    {
      { 1250, 550, 90 }, reset_flag_tower_down, 1 },
    {
      { 1250, 550, 90 }, close_door, 0 },
    {
      { 1250, 550, 0 }, NULL, 0 },
    {
      { 220, 775, 10 }, open_clap_arm, 0 },
    {
      { 220, 775, 0 }, NULL, 0 },
    {
      { 450, 775, 0 }, close_clap_arm, 0 },
    {
      { 700, 775, 0 }, open_clap_arm, 1 },
    {
      { 850, 775, 0 }, close_clap_arm, 0 },
    {
      { 600, 0, 0 }, NULL, 0 },
    {
      { 780, -40, 180 }, NULL, 0 },
    {
      { 500, 0, 0 }, NULL, 0 },
    {
      { 200, 0, 0 }, NULL, 0 },
    {
      { 275, 0, 0 }, set_release_left_cup, 0 },
    {
      { 275, 0, 0 }, set_release_right_cup, 0 } };

/*action_t route[NB_POSE] =
 {
 {
 { 500, 0, 0 }, NULL, 0 },
 {
 { 1064, 728, 60 }, set_flag_tower_down, 1 },
 {
 { 995, 586, 60 }, reset_flag_tower_down, 0 },
 {
 { 995, 586, 60 }, close_maintien, 0 },
 {
 { 170, 775, 0 }, open_clap_arm, 0 },
 {
 { 450, 775, 0 }, close_clap_arm, 0 },
 {
 { 700, 775, 0 }, open_clap_arm, 0 },
 {
 { 970, 775, 0 }, close_clap_arm, 0 } };*/

static uint8_t route_index;
static uint8_t route_retro;

pose_t route_update(void)
{
	pose_t pose_to_reach;
	func_cb_t route_function;

	if (controller_is_pose_reached(&controller)) {
		route_retro = 0;
		route_function = route[route_index].action_function;

		if (route_function)
			route_function();

		if (route_index < (NB_POSE - 1))
			route_index++;
	}

	pose_to_reach = route[route_index].p;

	/* mirror mode: invert path regarding bot's camp */
	if (detect_color()) {
		pose_to_reach.y *= -1;
		pose_to_reach.O *= -1;
	}

	return pose_to_reach;
}

void down_route_index()
{
	if ((route_index > 0) && (route_retro == 0)) {
		route_index--;
		route_retro = 1;
	}
}

#if 0
uint8_t get_can_retro()
{
	return route[route_index].can_retro;
}
#endif

uint8_t get_route_index()
{
	return route_index;
}
