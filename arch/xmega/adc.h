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

#include "utils.h"

void adc_setup(ADC_t *adc, func_cb_t callback);
void adc_read(ADC_t *adc, uint8_t pin);

#endif /* ADC_H_ */
