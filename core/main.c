#include "platform.h"
#include "sched.h"

int main(void)
{
	mach_setup();

	mach_tick_timer_setup();
	mach_sched_init();

#if defined(CONFIG_CALIBRATION)
	mach_check_calibration_mode();
#endif

	sched_enter_mainloop();

	/* we never reach this point */
	return 0;
}
