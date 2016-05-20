/*
 * sensor.c
 *
 *  Created on: 4 mai 2015
 *      Author: ldo
 */

#include <avr/io.h>

/**
 * 1 : devant droit PA6
 * 2 : devant gauche PA7 //pb
 * 3 : coté droit PA5
 * 4 : coté gauche PA4
 * 5 : derriere gauche (PA3)
 * 6 : derriere droite (PA2)
 *
 * 10 : bas droit (PJ1)
 * 11 : haut droit (PJ0)
 * 12 : bas gauche (PH7)
 * 13 : haut gauche (PH6)
 *
 * 15 : couleur (PH4)
 * 16 : tirette (PH3)
 * 17 : detect spot (PH2)
 * 18 : ascenseur bas (PH1)
 * 19 : ascenceur haut (PH0)
 */

/**
 *  PJ0 and PJ1
 */
uint8_t
detect_right_cup (void)
{
  uint8_t right_cup = PORTJ.IN;
  right_cup &= 0x03;
  if (right_cup == 0x01)
    {
      right_cup = 1;
    }
  else
    {
      right_cup = 0;
    }
  return right_cup;
}

/**
 * PH6 and PH7
 */
uint8_t
detect_left_cup (void)
{
  uint8_t left_cup = PORTH.IN;
  left_cup &= 0xC0;
  if (left_cup == 0x40)
    {
      left_cup = 1;
    }
  else
    {
      left_cup = 0;
    }
  return left_cup;
}

/**
 * PH0
 * \return if up is detected return 1 else return 0
 */
uint8_t
detect_elevator_up (void)
{
  uint8_t up = PORTH.IN;
  up &= 0x01;
  if (up == 0x01)
    {
      up = 0;
    }
  else
    {
      up = 1;
    }
  return up;
}

/**
 * PH1
 * \return if down is detected return 1 else return 0
 */
uint8_t
detect_elevator_down (void)
{
  uint8_t down = PORTH.IN;
  down &= 0x02;
  if (down == 0x02)
    {
      down = 0;
    }
  else
    {
      down = 1;
    }
  return down;
}

/**
 * PH2
 * \return if spot is detected return 1 else return 0
 */
uint8_t
detect_spot (void)
{
  uint8_t spot = PORTH.IN;
  spot &= 0x04;
  if (spot == 0x04)
    {
      spot = 0;
    }
  else
    {
      spot = 1;
    }
  return spot;
}

/**
 * PH3
 * \return if color is yellow return 1 else return 0
 */
uint8_t
detect_start (void)
{
  uint8_t start = PORTH.IN;
  start &= 0x08;
  if (start == 0x08)
    {
      start = 0;
    }
  else
    {
      start = 1;
    }
  return start;
}

/**
 * PH4
 * \return if color is yellow return 1 else return 0
 */
uint8_t
detect_color (void)
{
  uint8_t color = PORTH.IN;
  color &= 0x10;
  if (color == 0x10)
    {
      color = 0;
    }
  else
    {
      color = 1;
    }
  return color;
}
