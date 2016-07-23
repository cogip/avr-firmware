#include "irq.h"
#include "platform.h"
#include "sched.h"

int main(void)
{
	mach_setup();

	mach_tasks_init();
	mach_sched_init();

	mach_sched_run();

	/* we never reach this point */
	return 0;
}
