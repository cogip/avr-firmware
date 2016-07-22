#ifndef IRQ_H_
#define IRQ_H_

#define irq_enable() sei()
#define irq_disable() cli()

#define barrier()	__asm__ volatile("": : :"memory")

#endif /* IRQ_H_ */
