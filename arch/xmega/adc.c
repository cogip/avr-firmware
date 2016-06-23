/*
 * xmega_adc.c
 *
 *  Created on: 8 avr. 2015
 *      Author: ldo
 */

#include "adc.h"

static func_cb_t irq_handler;

/* Interrupt Service Routine for handling the ADC conversion complete IT */
ISR(ADCA_CH0_vect)
{
	if (irq_handler)
		irq_handler();

	/* Interrupt flag is cleared upon return from ISR */
}

/**
 * \fn
 * \brief
 * \param adc
 * single ended measurement
 * TODO param CH0 and PRPA
 */
void
xmega_adc_setup (ADC_t *adc, func_cb_t callback)
{
  /* Clear ADC bit in Power Reduction Port A Register */
  PR.PRPA &= ~0x02; // TODO

  /* set ADC prescaler */
  adc->PRESCALER = ADC_PRESCALER_DIV16_gc;
  /* set the ENABLE bit in CTRLA to enable ADC */
  adc->CTRLA = ADC_ENABLE_bm;
  /* configure 8-bits resolution in signed mode */
  adc->CTRLB = ADC_RESOLUTION_8BIT_gc | ADC_CONMODE_bm;
  //ADCB.CTRLB |= ADC_FREERUN_bm;

  /* ADC reference selection */
  adc->REFCTRL = ADC_REFSEL_VCC_gc; /*  VCC /1.6 = 2.0625 */

  /* ADC channel configuration */
  adc->CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc; // Single-ended positive input signal

  //  define the MUX selection for the positive ADC input
  adc->CH0.MUXCTRL = 00; //ADC_CH_MUXINT0_bm; // Connect PB1 to positive terminal

  /* ADC Interrupt configuration */
  // Ensure the conversion complete flag is cleared
  adc->INTFLAGS = ADC_CH0IF_bm; // 0x01
  /* Enable Conversion Complete Interrupt with low priority */
  adc->CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;
  // Enable interrupts
  sei ();
}

/**
 * TODO param CH0
 */
void
xmega_adc_read (ADC_t *adc, uint8_t pin)
{
  //  define the MUX selection for the positive ADC input
  adc->CH0.MUXCTRL &= 0x83;
  adc->CH0.MUXCTRL |= pin << 3;
  adc->CH0.CTRL |= ADC_CH_START_bm; // start conversion
}
