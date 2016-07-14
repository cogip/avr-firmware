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

typedef ADC_t adc_t;
typedef void (*adc_cb_t)(uint16_t, void *data);

void adc_setup(adc_t *adc, adc_cb_t callback, void *data);
void adc_async_read_start(adc_t *adc, uint8_t pin);

#endif /* ADC_H_ */
