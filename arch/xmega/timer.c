/**
 * \file timer.c
 *
 * \date 26 nov. 2014
 * \author ldo
 *
 * \brief XMEGA timer library
 *
 * The TC has six different modes of operation :
 * Normal mode
 * Frequency Generation mode
 * Single Slope PWM
 * Dual Slope PWM, overflow on TOP
 * Dual Slope PWM, overflow on TOP and BOTTOM.
 * Dual Slope PWM, overflow on BOTTOM
 */

#include <avr/io.h>
#include "timer.h"

#define IS_TIMER0(p)	((p) == &TCC0 || (p) == &TCD0 || \
			 (p) == &TCE0 || (p) == &TCF0)

#if defined(__AVR_ATxmega128A1__)
#define IS_TIMER1(p)	((p) == &TCC1 || (p) == &TCD1 || \
			 (p) == &TCE1 || (p) == &TCF1)
#elif  defined(__AVR_ATxmega256A3U__)
#define IS_TIMER1(p)	((p) == &TCC1 || (p) == &TCD1 || \
			 (p) == &TCE1)
#else
#error "MCU not supported"
#endif

static func_cb_t irq_tcc0_ovf_handler;
static func_cb_t irq_tcd0_ovf_handler;
static func_cb_t irq_tce0_ovf_handler;
static func_cb_t irq_tcf0_ovf_handler;

/* Timer 0 Overflow interrupt */
ISR(TCC0_OVF_vect)
{
	if (irq_tcc0_ovf_handler)
		irq_tcc0_ovf_handler();
}

ISR(TCD0_OVF_vect)
{
	if (irq_tcd0_ovf_handler)
		irq_tcd0_ovf_handler();
}

ISR(TCE0_OVF_vect)
{
	if (irq_tce0_ovf_handler)
		irq_tce0_ovf_handler();
}

ISR(TCF0_OVF_vect)
{
	if (irq_tcf0_ovf_handler)
		irq_tcf0_ovf_handler();
}

/**
 * In normal operation, the counter will count in the direction set by the
 * direction (DIR) bit for each clock until it reaches TOP or BOTTOM.
 * When up-counting, the counter counts from 0 to TOP (PER). When PER is
 * reached, the counter is reset to zero.
 * When down-counting, the counter counts from BOTTOM to 0. When 0 is reached,
 * the counter is set to BOTTOM.
 *
 * ATXMEGA128A1 use PORTC, PORTD, PORTE or PORTF. Each port has one Timer0 and
 * one Timer1.
 * Notation TCC0, TCC1, TCD0, TCD1, TCE0, TCE1, TCF0 and TCF1
 *
 */
void timer_normal_mode_setup(volatile timer_t *tc, uint16_t period,
			     tc_clksel_t clock_source, func_cb_t handler)
{
	volatile TC0_t *tc0;
	/*volatile TC1_t *tc1;*/

	if (IS_TIMER0(tc)) {
		tc0 = (TC0_t *)tc;

		if (handler) {
			if (tc0 == &TCC0)
				irq_tcc0_ovf_handler = handler;
			else if (tc0 == &TCD0)
				irq_tcd0_ovf_handler = handler;
			else if (tc0 == &TCE0)
				irq_tce0_ovf_handler = handler;
			else if (tc0 == &TCF0)
				irq_tcf0_ovf_handler = handler;
		}

		/* set the PER[H:L] register to select the timer period */
		tc0->PER = period; /* PER is TOP; */

		/* set the WGMODE[2:0] bits in CTRLB to select a normal mode */
		tc0->CTRLB = TC_WGMODE_NORMAL_gc;

		/* set the OVFINTLVL[1:0] bits in INTCTRLA to enable timer
		 * overflow interrupt
		 */
		tc0->INTCTRLA = TC_OVFINTLVL_LO_gc;

		/* start the TC by selecting a clock source
		 * (CLKSEL[3:0] in CTRLA)
		 */
		tc0->CTRLA = clock_source;
		tc0->CNT = 0;

	} else if (IS_TIMER1(tc)) {
		/*tc1 = (TC1_t *)tc;*/

		/* TODO */
	}
}

/**
 *  Configure TC as a quadrature counter
 */
void timer_qdec_mode_setup(volatile timer_t *tc,
			   tc_evsel_t event_channel,
			   const uint16_t line_count)
{
	volatile TC0_t *tc0;
	volatile TC1_t *tc1;

	if (IS_TIMER0(tc)) {
		tc0 = (TC0_t *)tc;

		tc0->CTRLD = TC_EVACT_QDEC_gc | event_channel;
		tc0->PER = (line_count * 4) - 1; /* full range, this is the low
						  * word of the 16 bit counter
						  */
		tc0->CTRLA = TC_CLKSEL_DIV1_gc;
		/*tc0->INTCTRLA = TC_OVFINTLVL_LO_gc;*/
		tc0->CNT = 0;

	} else if (IS_TIMER1(tc)) {
		tc1 = (TC1_t *)tc;

		tc1->CTRLD = TC_EVACT_QDEC_gc | event_channel;
		tc1->PER = (line_count * 4) - 1; /* full range, this is the low
						  * word of the 16 bit counter
						  */
		tc1->CTRLA = TC_CLKSEL_DIV1_gc;
		/* tc1->INTCTRLA = TC_OVFINTLVL_LO_gc; */
		tc1->CNT = 0;
	}
}

/**
 * For frequency generation the period time (T) is controlled by the CCA, CCB,
 * CCC or CCD registers.
 * The waveform generation (WG) output is toggled on each compare match between
 * the CNT and CCx registers
 */
void timer_pwm_mode_setup(volatile timer_t *tc, const uint8_t period,
			  tc_clksel_t prescaler)
{
	volatile TC0_t *tc0;
	/*volatile TC0_t *tc1;*/

	if (IS_TIMER0(tc)) {
		tc0 = (TC0_t *)tc;

		/* set the PER[H:L] register to select the timer period */
		tc0->PER = period;

		/* set the WGMODE[2:0] bits in CTRLB to select a waveform
		 * generation mode
		 */
		tc0->CTRLB = TC_WGMODE_SS_gc; /* single slope */

		/* start the TC by selecting a clock source
		 * (CLKSEL[3:0] in CTRLA)
		 */
		tc0->CTRLA = prescaler;

	} else if (IS_TIMER1(tc)) {
		/*tc1 = (TC1_t *)tc;*/

		/* TODO */
	}
}

/**
 *
 */
void timer_pwm_enable(volatile timer_t *tc, const uint8_t channel)
{
	volatile TC0_t *tc0;
	/*volatile TC0_t *tc1;*/

	if (IS_TIMER0(tc)) {
		tc0 = (TC0_t *)tc;

		/* set the CCxEN bit in CTRLB to enable Compare Channel x */
		switch (channel) {
		case 0:
			tc0->CTRLB |= (1 << TC0_CCAEN_bp);
			break;
		case 1:
			tc0->CTRLB |= (1 << TC0_CCBEN_bp);
			break;
		case 2:
			tc0->CTRLB |= (1 << TC0_CCCEN_bp);
			break;
		case 3:
			tc0->CTRLB |= (1 << TC0_CCDEN_bp);
			break;
		default:
			break;
		}

	} else if (IS_TIMER1(tc)) {
		/*tc1 = (TC1_t *)tc;*/

		/* TODO */
	}
}

/**
 *
 */
void timer_pwm_duty_cycle(volatile timer_t *tc, const uint8_t channel,
			  uint8_t duty_cycle)
{
	volatile TC0_t *tc0;
	/*volatile TC0_t *tc1;*/

	if (IS_TIMER0(tc)) {
		tc0 = (TC0_t *)tc;

		/* Write the new compare value to CCx[H:L] */
		switch (channel) {
		case 0:
			tc0->CCA = duty_cycle;
			break;
		case 1:
			tc0->CCB = duty_cycle;
			break;
		case 2:
			tc0->CCC = duty_cycle;
			break;
		case 3:
			tc0->CCD = duty_cycle;
			break;
		default:
			break;
		}

	} else if (IS_TIMER1(tc)) {
		/*tc1 = (TC1_t *)tc;*/

		/* TODO */
	}
}

inline uint16_t timer_get_cnt(volatile timer_t *tc)
{
	if (IS_TIMER0(tc))
		return ((TC0_t *)tc)->CNT;
	else if (IS_TIMER1(tc))
		return ((TC1_t *)tc)->CNT;
	else
		return 0;
}

inline void timer_set_cnt(volatile timer_t *tc, const uint16_t val)
{
	if (IS_TIMER0(tc))
		((TC0_t *)tc)->CNT = val;
	else if (IS_TIMER1(tc))
		((TC1_t *)tc)->CNT = val;
}
