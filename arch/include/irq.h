#ifndef IRQ_H_
#define IRQ_H_

#if defined(__AVR__)
#include <avr/interrupt.h>

#define irq_enable() sei()
#define irq_disable() cli()

#else

#define irq_enable()
#define irq_disable()

#define ISR(_f) void _irq_##_f()

#endif

#define barrier()	__asm__ volatile("": : :"memory")

#endif /* IRQ_H_ */
