#ifndef TIMER_H_
#define TIMER_H_

#if defined(__AVR__)
#include <avr/io.h>
#include <avr/interrupt.h>
#endif
#include <stdint.h>

#include "utils.h"

#if defined(__AVR__)
typedef void hwtimer_t;
typedef TC_CLKSEL_t tc_clksel_t;
typedef TC_EVSEL_t tc_evsel_t;
#else
typedef int16_t hwtimer_t;
typedef uint16_t tc_clksel_t;
typedef void *tc_evsel_t;

int16_t _timers_[5];

typedef int16_t TC0_t;
typedef int16_t TC1_t;

#define TCC0 (_timers_[0])
#define TCD0 (_timers_[1])
#define TCE0 (_timers_[2])
#define TCE1 (_timers_[3])
#define TCF0 (_timers_[4])

#define TC_EVSEL_CH0_gc ((tc_evsel_t) 0)
#define TC_EVSEL_CH2_gc ((tc_evsel_t) 2)

#define TC_CLKSEL_DIV8_gc 8
#define TC_CLKSEL_DIV1024_gc 1024
#endif

void timer_normal_mode_setup(volatile hwtimer_t *tc, uint16_t period,
			     tc_clksel_t clock_source, func_cb_t handler);

void timer_qdec_mode_setup(volatile hwtimer_t *tc, tc_evsel_t event_channel,
			   const uint16_t line_count);

void timer_pwm_mode_setup(volatile hwtimer_t *tc, const uint8_t period,
			  tc_clksel_t prescaler);

void timer_pwm_enable(volatile hwtimer_t *tc, const uint8_t channel);

void timer_pwm_duty_cycle(volatile hwtimer_t *tc, const uint8_t channel,
			  uint8_t duty_cycle);

uint16_t timer_get_cnt(volatile hwtimer_t *tc);

void timer_set_cnt(volatile hwtimer_t *tc, const uint16_t val);


#endif /* TIMER_H_ */
