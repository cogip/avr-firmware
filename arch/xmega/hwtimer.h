/*
 * timer.h
 *
 *  Created on: 26 nov. 2014
 *      Author: ldo
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#include "utils.h"

typedef void hwtimer_t;
typedef TC_CLKSEL_t tc_clksel_t;
typedef TC_EVSEL_t tc_evsel_t;

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
