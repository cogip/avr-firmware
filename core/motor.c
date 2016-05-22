/*
 * motor.c
 *
 *  Created on: 25 avr. 2015
 *      Author: ldo
 */

#include <math.h>
#include <xmega/timer.h>

#include "action.h"
#include "motor.h"
#include "route.h"

void
motor_drive (polar_t command)
{
  /************************ commandes moteur ************************/
  int16_t right_command = (int16_t) (command.distance + command.angle);
  int16_t left_command = (int16_t) (command.distance - command.angle);

  /*uint8_t side_irs[] =
   { 0, 1 };
   uint8_t rear_irs[] =
   { 2, 3 };
   uint8_t front_irs[] =
   { 4, 5, 6, 7 };

  uint8_t stop = 0;

  if ((right_command > 0) && (left_command > 0))
    stop = stop_robot (front_irs, 4);
  else if ((right_command < 0) && (left_command < 0))
    stop = stop_robot (rear_irs, 2);
  else
    stop = stop_robot (side_irs, 2);

  if (stop && get_route_index ())
    {
      right_command = 0;
      left_command = 0;
      if ((stop > 1) && (get_can_retro ()))
	down_route_index ();
    }*/

  right_motor_drive (right_command);
  left_motor_drive (left_command);
}

#define MIN_PWM 30

/** limite la commande de vitesse
 * @param value from -16535 to 16535
 * @return pwm value from 0 to max */
uint8_t
pwm_limitation (int16_t value, uint8_t max)
{
  /* positive value */
  if (value > 0)
    {
      /*if (value < MIN_PWM)
       {
       value = MIN_PWM;
       }*/
      if (value > max)
	{
	  value = max;
	}
    }
  /* negative value */
  else
    {
      /*if (value > -MIN_PWM)
       {
       value = -MIN_PWM;
       }*/
      if (value < -max)
	{
	  value = -max;
	}

    }
  return fabs (value);
}

/**
 * \param pwm value for motor (signed value)
 * */
void
left_motor_drive (int16_t pwm)
{
  /* limitation de la commande de vitesse */
  uint8_t pwm_limit = pwm_limitation (pwm, 200);

  /* advance */
  if (pwm > 0)
    {
      PORTD.OUTSET = PIN4_bm;
    }
  else
    {
      PORTD.OUTCLR = PIN4_bm;
    }

  /* generate PWM */
  xmega_timer_0_pwm_duty_cycle (&TCE0, 0, pwm_limit);
}

/**
 * \param pwm value for motor (signed value)
 * */
void
right_motor_drive (int16_t pwm)
{
  /* limitation de la commande de vitesse */
  uint8_t pwm_limit = pwm_limitation (pwm, 200);

  /* advance */
  if (pwm > 0)
    {
      PORTD.OUTCLR = PIN5_bm;
    }
  else
    {
      PORTD.OUTSET = PIN5_bm;
    }

  /* generate PWM */
  xmega_timer_0_pwm_duty_cycle (&TCE0, 1, pwm_limit);
}
