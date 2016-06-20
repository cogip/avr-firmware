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
uint8_t detect_right_cup(void)
{
	return (PORTJ.IN & 0x03) == 0x01;
}

/**
 * PH6 and PH7
 */
uint8_t detect_left_cup(void)
{
	return (PORTH.IN & 0xC0) == 0x40;
}

/**
 * PH0
 * \return if up is detected return 1 else return 0
 */
uint8_t detect_elevator_up(void)
{
	return !(PORTH.IN & 0x01);
}

/**
 * PH1
 * \return if down is detected return 1 else return 0
 */
uint8_t detect_elevator_down(void)
{
	return !(PORTH.IN & 0x02);
}

/**
 * PH2
 * \return if spot is detected return 1 else return 0
 */
uint8_t detect_spot(void)
{
	return !(PORTH.IN & 0x04);
}

/**
 * PH3
 */
uint8_t detect_start(void)
{
	return !(PORTH.IN & 0x08);
}

/**
 * PH4
 * \return if color is yellow return 1 else return 0
 */
uint8_t detect_color(void)
{
	return !(PORTH.IN & 0x10);
}
