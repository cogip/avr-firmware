/*
 * encoder.c
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

//#include <xmega_usart.h>
#include "encoder.h"

int16_t
decode (int16_t counter)
{
  if (counter > 1000)
    {
      counter -= 2000;
    }
  if (counter < -1000)
    {
      counter += 2000;
    }
  return counter;
}

/**
 *
 */
polar
read_encoder (void)
{
  polar robot_speed;

  int16_t left_speed = decode (TCE1.CNT);
  int16_t right_speed = decode (-TCF0.CNT);

  /*xmega_usart_transmit (&USARTC0, (int8_t) (left_speed >> 8));
   xmega_usart_transmit (&USARTC0, (int8_t) left_speed);
   xmega_usart_transmit (&USARTC0, (int8_t) (right_speed >> 8));
   xmega_usart_transmit (&USARTC0, (int8_t) right_speed);*/

  TCE1.CNT = 0;
  TCF0.CNT = 0;

  /* update speed */
  robot_speed.distance = (right_speed + left_speed) / 2.0;
  robot_speed.angle = right_speed - left_speed;

  /*xmega_usart_transmit (&USARTC0, (int8_t) (robot_speed.distance >> 8));
   xmega_usart_transmit (&USARTC0, (int8_t) robot_speed.distance);
   xmega_usart_transmit (&USARTC0, (int8_t) (robot_speed.angle >> 8));
   xmega_usart_transmit (&USARTC0, (int8_t) robot_speed.angle);*/

  return robot_speed;
}

/*int16_t
 read_encoder_elevator (void)
 {
 int16_t speed_elevator = decode (TCF1.CNT);
 TCF1.CNT = 0;
 return speed_elevator;
 }*/
