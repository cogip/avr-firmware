#include "qdec.h"
#include "xmega/timer.h"

static int8_t qdec_setup_pinsense(qdec_t *qdec, uint8_t pin)
{
	uint8_t pinctrl = PORT_ISC_LEVEL_gc | PORT_OPC_PULLUP_gc;

	switch (pin) {
	case PIN0_bp:
		qdec->pin_port->PIN0CTRL = pinctrl;
		break;
	case PIN1_bp:
		qdec->pin_port->PIN1CTRL = pinctrl;
		break;
	case PIN2_bp:
		qdec->pin_port->PIN2CTRL = pinctrl;
		break;
	case PIN3_bp:
		qdec->pin_port->PIN3CTRL = pinctrl;
		break;
	case PIN4_bp:
		qdec->pin_port->PIN4CTRL = pinctrl;
		break;
	case PIN5_bp:
		qdec->pin_port->PIN5CTRL = pinctrl;
		break;
	case PIN6_bp:
		qdec->pin_port->PIN6CTRL = pinctrl;
		break;
	case PIN7_bp:
		qdec->pin_port->PIN7CTRL = pinctrl;
		break;
	default:
		return -1;
	}

	return 0;
}

int8_t qdec_setup(qdec_t *qdec)
{
	EVSYS_CHMUX_t chmux = EVSYS_CHMUX_OFF_gc;

	/* configure A and B pins as input */
	qdec->pin_port->DIRCLR = (1 << qdec->pin_qdph0);
	qdec->pin_port->DIRCLR = (1 << qdec->pin_qdph90);

	/* configure low level sense */
	if (qdec_setup_pinsense(qdec, qdec->pin_qdph0))
		return -1;
	if (qdec_setup_pinsense(qdec, qdec->pin_qdph90))
		return -1;

	/* compute event channel mux */
	if (qdec->pin_port == &PORTA)
		chmux = ((uint8_t) EVSYS_CHMUX_PORTA_PIN0_gc + qdec->pin_qdph0);
	else if (qdec->pin_port == &PORTB)
		chmux = ((uint8_t) EVSYS_CHMUX_PORTB_PIN0_gc + qdec->pin_qdph0);
	else if (qdec->pin_port == &PORTC)
		chmux = ((uint8_t) EVSYS_CHMUX_PORTC_PIN0_gc + qdec->pin_qdph0);
	else if (qdec->pin_port == &PORTD)
		chmux = ((uint8_t) EVSYS_CHMUX_PORTD_PIN0_gc + qdec->pin_qdph0);
	else if (qdec->pin_port == &PORTE)
		chmux = ((uint8_t) EVSYS_CHMUX_PORTE_PIN0_gc + qdec->pin_qdph0);
	else if (qdec->pin_port == &PORTF)
		chmux = ((uint8_t) EVSYS_CHMUX_PORTF_PIN0_gc + qdec->pin_qdph0);
	else
		return -1;

	/* event configuration */
	switch (qdec->event_channel) {
	default:
	case TC_EVSEL_OFF_gc:
	case TC_EVSEL_CH1_gc:
	case TC_EVSEL_CH3_gc:
	case TC_EVSEL_CH5_gc:
	case TC_EVSEL_CH6_gc:
	case TC_EVSEL_CH7_gc:
		/* qdec not supported on these channel */
		return -1;

	case TC_EVSEL_CH0_gc:
		EVSYS.CH0MUX = chmux;
		EVSYS.CH0CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;
		break;

	case TC_EVSEL_CH2_gc:
		EVSYS.CH2MUX = chmux;
		EVSYS.CH2CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;
		break;

	case TC_EVSEL_CH4_gc:
		EVSYS.CH4MUX = chmux;
		EVSYS.CH4CTRL = EVSYS_QDEN_bm
			| EVSYS_DIGFILT_2SAMPLES_gc /*| EVSYS_QDIEN_bm*/;
		break;
	}

	timer_qdec_mode_setup(qdec->tc, qdec->event_channel, qdec->line_count);
	timer_set_cnt(qdec->tc, qdec->line_count >> 1);

	return 0;
}

/* Counter value is between [0..ENCODER_RES] at hardware stage,
 * Qdec driver shifts its zero reference to ENCODER_RES/2 (as it counter is
 * working on unsigned type) each time the counter is read.
 *
 * This function translate [0..ENCODER_RES/2] range to backward direction
 * all value in [ENCODER_RES/2..ENCODER_RES] are considered as forward
 * direction.
 */
static int16_t decode(qdec_t *qdec, const uint16_t counter)
{
	int16_t signed_value = (int16_t) counter;

	signed_value -= qdec->line_count >> 1;

	return qdec->polarity * signed_value;
}

int16_t qdec_read(qdec_t *qdec)
{
	int16_t value;
	uint16_t unsigned_value;

	/* TODO: both following lines should be in a critical section */
	unsigned_value = timer_get_cnt(qdec->tc);
	timer_set_cnt(qdec->tc, qdec->line_count >> 1);

	value = decode(qdec, unsigned_value);

	return value;
}
