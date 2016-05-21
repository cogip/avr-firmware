#ifndef AUTOCONF_GCC_AVR_H
#define AUTOCONF_GCC_AVR_H
#include <generated/autoconf.h>

#ifndef CONFIG_F_CPU
#error "MCU freq must be defined. run 'make menuconfig'"
#endif

#define F_CPU CONFIG_F_CPU

#endif
