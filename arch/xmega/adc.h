/*
 * adc.h
 *
 *  Created on: 29 nov. 2014
 *      Author: ldo
 */

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * \fn
 * \brief
 * \param
 */
void
xmega_adc_setup (ADC_t *adc);

void
xmega_adc_read (ADC_t *adc, uint8_t pin);

#endif /* ADC_H_ */
