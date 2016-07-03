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

typedef void (*adc_cb_t)(uint16_t);

void adc_setup(ADC_t *adc, adc_cb_t callback);
void adc_async_read_start(ADC_t *adc, uint8_t pin);

#endif /* ADC_H_ */
