/*
 * route.c
 *
 *  Created on: 2 mai 2015
 *      Author: ldo
 */

#include "route.h"
#include "action.h"

#define NB_POSE 21
// 10610 pour 1 m - 4650 pour 1/4 tour 90°

uint8_t flag_tower_down = 0;

void
set_flag_tower_down (void)
{
  flag_tower_down = 1;
}

void
reset_flag_tower_down (void)
{
  flag_tower_down = 0;
}

action route[NB_POSE] =
  {
    {
      { 2500, 0, 0 }, NULL, 0, 0 },
    {
      { 500, 0, 0 }, NULL, 0, 0 },
    {
      { 820, 290, 5 }, NULL, 0, 1 },
    {
      { 1185, 370, 110 }, NULL, 0, 0 },
    {
      { 1115, 670, 0 }, NULL, 0, 0 },
    {
      { 1250, 670, 90 }, NULL, 0, 0 },
    {
      { 1250, 700, 90 }, set_flag_tower_down, 0, 0 },
    {
      { 1250, 550, 90 }, reset_flag_tower_down, 0, 1 },
    {
      { 1250, 550, 90 }, close_door, 0, 0 },
    {
      { 1250, 550, 0 }, NULL, 0, 0 },
    {
      { 220, 775, 10 }, open_clap_arm, 0, 0 },
    {
      { 220, 775, 0 }, NULL, 0, 0 },
    {
      { 450, 775, 0 }, close_clap_arm, 0, 0 },
    {
      { 700, 775, 0 }, open_clap_arm, 0, 1 },
    {
      { 850, 775, 0 }, close_clap_arm, 0, 0 },
    {
      { 600, 0, 0 }, NULL, 0, 0 },
    {
      { 780, -40, 180 }, NULL, 0, 0 },
    {
      { 500, 0, 0 }, NULL, 0, 0 },
    {
      { 200, 0, 0 }, NULL, 0, 0 },
    {
      { 275, 0, 0 }, set_release_left_cup, 0, 0 },
    {
      { 275, 0, 0 }, set_release_right_cup, 0, 0 } };

/*action route[NB_POSE] =
 {
 {
 { 500, 0, 0 }, NULL, 0, 0 },
 {
 { 1064, 728, 60 }, set_flag_tower_down, 0, 1 },
 {
 { 995, 586, 60 }, reset_flag_tower_down, 0, 0 },
 {
 { 995, 586, 60 }, close_maintien, 0, 0 },
 {
 { 170, 775, 0 }, open_clap_arm, 0, 0 },
 {
 { 450, 775, 0 }, close_clap_arm, 0, 0 },
 {
 { 700, 775, 0 }, open_clap_arm, 0, 0 },
 {
 { 970, 775, 0 }, close_clap_arm, 0, 0 } };*/

uint8_t route_index = 0;
uint8_t route_retro = 0;
uint8_t
(*route_function) (void);

pose
route_update (void)
{
  if (pose_reached == 1)
    {
      route_retro = 0;
      route_function = route[route_index].action_function;
      if (route_function)
	route[route_index].status = route_function ();
      if (route_index < (NB_POSE - 1))
	{
	  route_index++;
	}
      pose_reached = 0;
    }
  return route[route_index].p;
}

void
down_route_index ()
{
  if ((route_index > 0) && (route_retro == 0))
    {
      route_index--;
      route_retro = 1;
    }
}

uint8_t
get_can_retro ()
{
  return route[route_index].can_retro;
}

uint8_t
get_route_index ()
{
  return route_index;
}
