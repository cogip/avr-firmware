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

#define IS_TIMER0(p)	((p) == &TCC0 || (p) == &TCD0 || \
			 (p) == &TCE0 || (p) == &TCF0)

#define IS_TIMER1(p)	((p) == &TCC1 || (p) == &TCD1 || \
			 (p) == &TCE1 || (p) == &TCF1)


void timer_0_normal_mode_setup(volatile TC0_t *tc, uint16_t period,
			       TC_CLKSEL_t clock_source);

void timer_0_qdec_mode_setup(volatile TC0_t *tc, TC_EVSEL_t event_channel,
			     const uint16_t line_count);

void timer_1_qdec_mode_setup(volatile TC1_t *tc, TC_EVSEL_t event_channel,
			     const uint16_t line_count);

void timer_0_pwm_mode_setup(volatile TC0_t *tc, const uint8_t period,
			    TC_CLKSEL_t prescaler);

void timer_0_pwm_enable(volatile TC0_t *tc, const uint8_t channel);

void timer_0_pwm_duty_cycle(volatile TC0_t *tc, const uint8_t channel,
			    uint8_t duty_cycle);

void timer_0_register_ovf_cb(func_cb_t handler);

#endif /* TIMER_H_ */
