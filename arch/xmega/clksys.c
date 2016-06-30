#include <avr/io.h>

#include "clksys.h"

void clksys_intrc_32MHz_setup(void)
{
	/* Configuration change protection : Protected IO register
	 * disable automatically all interrupts for the next
	 * four CPU instruction cycles
	 */
	CCP = CCP_IOREG_gc;

	/* Oscillator : 32MHz Internal Oscillator Enable */
	OSC.CTRL = OSC_RC32MEN_bm;

	/* Wait for the internal 32 MHz RC oscillator to stabilize */
	while (!(OSC.STATUS & OSC_RC32MRDY_bm))
		;

	/* Configuration change protection : Protected IO register */
	CCP = CCP_IOREG_gc;

	/* System Clock Selection : 32MHz Internal Oscillator */
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;

	/* Note:
	 * Prescaler A, B & C are 0x0 by default, thus no division. So:
	 *   ClkSys == 32MHz
	 *   ClkPer4 == ClkPer2 == ClkPer == 32MHz
	 */
}


