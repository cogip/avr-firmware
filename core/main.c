/*
   Copyright 2015, 2016 Yannick Gicquel <ygicquel@gmail.com>

   This file is part of Cogip AVR Firmware.

   Cogip AVR Firmware is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Cogip AVR Firmware is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Cogip AVR Firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "platform.h"

int main(void)
{
	mach_setup();

	mach_tasks_init();
	mach_sched_init();

	mach_sched_run();

	/* we never reach this point */
	return 0;
}
