#ifndef IRQ_H_
#define IRQ_H_

#define irq_enable()
#define irq_disable()

#define ISR(_f) void _irq_##_f()

#define barrier()	__asm__ volatile("": : :"memory")

#endif /* IRQ_H_ */
