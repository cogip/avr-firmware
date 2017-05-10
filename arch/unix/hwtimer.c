/**
 * The TC has six different modes of operation :
 * Normal mode
 * Frequency Generation mode
 * Single Slope PWM
 * Dual Slope PWM, overflow on TOP
 * Dual Slope PWM, overflow on TOP and BOTTOM.
 * Dual Slope PWM, overflow on BOTTOM
 */

//#include <avr/io.h>
#define _POSIX_C_SOURCE 200200L
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#include "console.h"
#include "irq.h"
#include "hwtimer.h"
#include "platform.h"

//#define hwtimer_t avrhwtimer_t

#define IS_TIMER0(p)	((p) == &TCC0 || (p) == &TCD0 || \
			 (p) == &TCE0 || (p) == &TCF0)

#if defined(__AVR_ATxmega128A1__)
#define IS_TIMER1(p)	((p) == &TCC1 || (p) == &TCD1 || \
			 (p) == &TCE1 || (p) == &TCF1)
#elif defined(__AVR_ATxmega128A3U__) || defined(__AVR_ATxmega256A3U__)
#define IS_TIMER1(p)	((p) == &TCC1 || (p) == &TCD1 || \
			 (p) == &TCE1)
#else
//#error "MCU not supported"
#define IS_TIMER1(p)	(0)
#endif

static func_cb_t irq_tcc0_ovf_handler;
static func_cb_t irq_tcd0_ovf_handler;
static func_cb_t irq_tce0_ovf_handler;
static func_cb_t irq_tcf0_ovf_handler;
struct timespec orig_time;

/* Timer 0 Overflow interrupt */
ISR(TCC0_OVF_vect)
{
	clock_gettime(CLOCK_REALTIME, &orig_time);
	if (irq_tcc0_ovf_handler)
		irq_tcc0_ovf_handler();
}

ISR(TCD0_OVF_vect)
{
	clock_gettime(CLOCK_REALTIME, &orig_time);
	if (irq_tcd0_ovf_handler)
		irq_tcd0_ovf_handler();
}

ISR(TCE0_OVF_vect)
{
	clock_gettime(CLOCK_REALTIME, &orig_time);
	if (irq_tce0_ovf_handler)
		irq_tce0_ovf_handler();
}

ISR(TCF0_OVF_vect)
{
	clock_gettime(CLOCK_REALTIME, &orig_time);
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
void timer_normal_mode_setup(volatile hwtimer_t *tc, uint16_t period,
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

#if 0
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
#else
	{
		uint16_t period_ms = 0;

		timer_t timerid;
		struct sigevent sev;
		struct itimerspec its;
		struct sigaction sa;


		cons_printf("%s: period = %d\n", __func__, period);
		switch (period) {
		case 325:
#if defined(CONFIG_SIMU_NORMAL_SPEED)
			period_ms = 10;
#else /* SIMU_10X_SPEED */
			period_ms = 1;
#endif
			break;
		default:
			cons_printf("not supported\n");
			break;
		}

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                       } while (0)

           /* Establish handler for timer signal */

           cons_printf("Establishing handler for signal %d\n", SIGRTMIN);
           sa.sa_flags = SA_SIGINFO;
		if (handler) {
			if (tc0 == &TCC0)
				sa.sa_sigaction = _irq_TCC0_OVF_vect;
			else if (tc0 == &TCD0)
				sa.sa_sigaction = _irq_TCD0_OVF_vect;
			else if (tc0 == &TCE0)
				sa.sa_sigaction = _irq_TCE0_OVF_vect;
			else if (tc0 == &TCF0)
				sa.sa_sigaction = _irq_TCF0_OVF_vect;
		}

           sigemptyset(&sa.sa_mask);
           if (sigaction(SIGRTMIN, &sa, NULL) == -1)
               errExit("sigaction");

           ///* Block timer signal temporarily */

           //printf("Blocking signal %d\n", SIGRTMIN);
           //sigemptyset(&mask);
           //sigaddset(&mask, SIGRTMIN);
           //if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
           //    errExit("sigprocmask");

           /* Create the timer */

           sev.sigev_notify = SIGEV_SIGNAL;
           sev.sigev_signo = SIGRTMIN;
           sev.sigev_value.sival_ptr = &timerid;
           if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
               errExit("timer_create");

           cons_printf("timer ID is 0x%lx\n", (long) timerid);

           /* Start the timer */

           //freq_nanosecs = atoll(argv[2]);
           //its.it_value.tv_sec = freq_nanosecs / 1000000000;
           //its.it_value.tv_nsec = freq_nanosecs % 1000000000;
           its.it_value.tv_sec = 0;
           its.it_value.tv_nsec = period_ms * 1000 * 1000;
           its.it_interval.tv_sec = its.it_value.tv_sec;
           its.it_interval.tv_nsec = its.it_value.tv_nsec;

	   clock_gettime(CLOCK_REALTIME, &orig_time);
           if (timer_settime(timerid, 0, &its, NULL) == -1)
                errExit("timer_settime");

	}
#endif

	} else if (IS_TIMER1(tc)) {
		/*tc1 = (TC1_t *)tc;*/

		/* TODO */
	}
}

/**
 *  Configure TC as a quadrature counter
 */
void timer_qdec_mode_setup(volatile hwtimer_t *tc,
			   tc_evsel_t event_channel,
			   const uint16_t line_count)
{
#if 0
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
#endif
}

/**
 * For frequency generation the period time (T) is controlled by the CCA, CCB,
 * CCC or CCD registers.
 * The waveform generation (WG) output is toggled on each compare match between
 * the CNT and CCx registers
 */
void timer_pwm_mode_setup(volatile hwtimer_t *tc, const uint8_t period,
			  tc_clksel_t prescaler)
{
#if 0
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
#endif
}

/**
 *
 */
void timer_pwm_enable(volatile hwtimer_t *tc, const uint8_t channel)
{
#if 0
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
#endif
}

/**
 *
 */
int8_t simu_left_motor_dir = 0;
int8_t simu_right_motor_dir = 0;

#define SIMU_ENC_BUFSIZE	3
int16_t simu_left_motor_encoder[SIMU_ENC_BUFSIZE] = {0,};
int16_t simu_right_motor_encoder[SIMU_ENC_BUFSIZE] = {0,};

void timer_pwm_duty_cycle(volatile hwtimer_t *tc, const uint8_t channel,
			  uint8_t duty_cycle)
{
#if 0
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
#else
	int16_t i = 0, s = 0;
	hwtimer_t *enc_timer;

	if (tc == hbridges.tc && channel == hbridges.engines[HBRIDGE_MOTOR_LEFT].pwm_channel) {
		/* left motor */
		enc_timer = encoders[0].tc;
		*enc_timer = simu_left_motor_dir ? duty_cycle : -duty_cycle;
		*enc_timer += ((WHEELS_ENCODER_RESOLUTION / 4) >> 1);

		for (i = 0; i < SIMU_ENC_BUFSIZE - 1; i++) {
			s += simu_left_motor_encoder[i];
			simu_left_motor_encoder[i] = simu_left_motor_encoder[i+1];
		}
		s = (s + simu_left_motor_encoder[i] + *enc_timer) / (SIMU_ENC_BUFSIZE + 1);

		simu_left_motor_encoder[i] = s;

	} else if (tc == hbridges.tc && channel == hbridges.engines[HBRIDGE_MOTOR_RIGHT].pwm_channel) {
		/* right motor */
		enc_timer = encoders[1].tc;
		*enc_timer = simu_right_motor_dir ? -duty_cycle : duty_cycle;
		*enc_timer += ((WHEELS_ENCODER_RESOLUTION / 4) >> 1);

		for (i = 0; i < SIMU_ENC_BUFSIZE - 1; i++) {
			s += simu_right_motor_encoder[i];
			simu_right_motor_encoder[i] = simu_right_motor_encoder[i+1];
		}
		s = (s + simu_right_motor_encoder[i] + *enc_timer) / (SIMU_ENC_BUFSIZE + 1);

		simu_right_motor_encoder[i] = s;

	} else {
		cons_printf("%s: unsupported, channel = %d\n", __func__, channel);
	}
#endif
}

inline uint16_t timer_get_cnt(volatile hwtimer_t *tc)
{
#if 0
	if (IS_TIMER0(tc))
		return ((TC0_t *)tc)->CNT;
	else if (IS_TIMER1(tc))
		return ((TC1_t *)tc)->CNT;
	else
		return 0;
#else
	if (tc == &TCC0) {
		struct timespec current_time;

		clock_gettime(CLOCK_REALTIME, &current_time);
		long delta_ms = orig_time.tv_sec * 1000 - orig_time.tv_sec * 1000;
		delta_ms += (orig_time.tv_nsec - current_time.tv_nsec) / 1000;
		if (delta_ms < 0) delta_ms *= -1;
		delta_ms *= 645; //645 == 20 ms

		//printf("%ld\n", delta_ms);
		return delta_ms;
	} else if (tc == encoders[0].tc) {
		return simu_left_motor_encoder[SIMU_ENC_BUFSIZE-1];
	} else if (tc == encoders[1].tc) {
		return simu_right_motor_encoder[SIMU_ENC_BUFSIZE-1];
	} else {
		cons_printf("%s: unsupported\n", __func__);
	}

	return 0;
#endif
}

inline void timer_set_cnt(volatile hwtimer_t *tc, const uint16_t val)
{
#if 0
	if (IS_TIMER0(tc))
		((TC0_t *)tc)->CNT = val;
	else if (IS_TIMER1(tc))
		((TC1_t *)tc)->CNT = val;
#endif
	if (tc == encoders[0].tc)
		*encoders[0].tc = val;
	if (tc == encoders[1].tc)
		*encoders[1].tc = val;
}
