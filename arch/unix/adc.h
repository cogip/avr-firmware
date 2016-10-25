#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
//#include <avr/io.h>
//#include <avr/interrupt.h>

//typedef ADC_t adc_t;
typedef void *adc_t;
typedef void (*adc_cb_t)(uint16_t, void *data);

char _adc_[2];
#define ADCA (_adc_[0])
#define ADCB (_adc_[1])

void adc_setup(adc_t *adc, adc_cb_t callback, void *data);
void adc_async_read_start(adc_t *adc, uint8_t pin);

#endif /* ADC_H_ */
