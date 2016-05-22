/*
 * analog_sensor.c
 *
 *  Created on: 10 avr. 2015
 *      Author: ldo
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "analog_sensor.h"

//static volatile uint8_t distance[8];
static volatile uint8_t distance[8];
static volatile uint8_t sensor_index = 0;

static volatile uint8_t adc_flag = 0;
static volatile uint16_t adc_result;

// Interrupt Service Routine for handling the ADC conversion complete interrupt
ISR(ADCA_CH0_vect)
{
  adc_result = ADCA.CH0.RES;
  //distance[sensor_index][value_index] = gp2y0a41_read (adc_result); //gp2d120_read(adc_result);
  //distance[sensor_index][value_index] = 0;//gp2d120_read (adc_result);

  adc_flag = 1;

  // Interrupt flag is cleared upon return from ISR
}

/*
 * @return distance in cm
 * */
uint8_t
gp2d120_read (uint16_t adc)
{
  //uint16_t adc = adc_read(pin);
  double voltage = adc * 3.3 / 255; /* 8-bits conversion - Vcc = 3.3V */
  //double d = voltage * 0.0833 - 0.0016; // Vcc = 5V
  double d = voltage * 0.126 - 0.007; // Vcc = 3.3V
  double distance = 1 / d;

  if ((distance < GP2D120_DIST_MIN) || (distance > GP2D120_DIST_MAX))
    {
      return 0;
    }
  else
    return distance;
}

/*
 * @return distance in cm
 * */
uint8_t
gp2y0a41_read (uint16_t adc)
{
  //uint16_t adc = adc_read(pin);
  double voltage = adc * 3.3 / 255; /* 8-bits conversion - Vcc = 3.3V */
  //double d = voltage * 0.0833 - 0.0016; // Vcc = 5V
  double d = voltage * 0.1275 - 0.0253; // Vcc = 3.3V
  double distance = 1 / d;

  if ((distance < GP2Y0A41_DIST_MIN) || (distance > GP2Y0A41_DIST_MAX))
    {
      return 0;
    }
  else
    return (uint8_t) distance;
}

/*
 * @return distance in cm
 * */
uint8_t
gp2y0a21_read (uint16_t adc)
{
  //uint16_t adc = adc_read(pin);
  double voltage = adc * 3.3 / 255; /* 8-bits conversion - Vcc = 3.3V */
  //double d = voltage * 0.0833 - 0.0016; // Vcc = 5V
  double d = voltage * 0.045 - 0.01; // Vcc = 3.3V
  double distance = 1 / d;

  if ((distance < GP2Y0A21_DIST_MIN) || (distance > GP2Y0A21_DIST_MAX))
    {
      return 0;
    }
  else
    return (uint8_t) distance;
}

void
read_analog_sensor (void/*uint16_t adc_result*/)
{
  if (adc_flag == 1)
    {
      /*xmega_usart_transmit (&USARTC0, 0xAA);
       xmega_usart_transmit (&USARTC0, 0xAA);
       xmega_usart_transmit (&USARTC0, 0x00);
       xmega_usart_transmit (&USARTC0, distance[7][value_index]);*/
      distance[sensor_index] = gp2y0a21_read (adc_result); //gp2d120_read(adc_result);

      sensor_index++;
      if (sensor_index > 7)
	{
	  sensor_index = 0;

	}

      xmega_adc_read (&ADCA, sensor_index);
      //detect_obstacle();

      adc_flag = 0;
    }
}

uint8_t
detect_obstacle (uint8_t *ir_ids, uint8_t ir_nb)
{
  uint8_t stop = 0;
  uint8_t i;
  for (i = 0; i < ir_nb; i++)
    {
      if ((distance[ir_ids[i]] < 20) && (distance[ir_ids[i]] != 0))
	{
	  stop = 1;
	  /*xmega_usart_transmit (&USARTC0, 0xCC);
	   xmega_usart_transmit (&USARTC0, 0xCC);
	   xmega_usart_transmit (&USARTC0, i);
	   xmega_usart_transmit (&USARTC0, distance_filtered);*/
	}
    }
  return stop;
}
