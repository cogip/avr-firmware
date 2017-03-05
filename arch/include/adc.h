#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#if defined(__AVR__)
#include <avr/io.h>
#include <avr/interrupt.h>

typedef ADC_t adc_t;
#else
typedef void *adc_t;

char _adc_[2];
#define ADCA (_adc_[0])
#define ADCB (_adc_[1])
#endif
typedef void (*adc_cb_t)(uint16_t, void *data);

void adc_setup(adc_t *adc, adc_cb_t callback, void *data);
void adc_async_read_start(adc_t *adc, uint8_t pin);

#endif /* ADC_H_ */
