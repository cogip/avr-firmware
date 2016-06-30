src-y		+= arch/xmega/adc.c
src-y		+= arch/xmega/clksys.c
src-y		+= arch/xmega/qdec.c
src-y		+= arch/xmega/timer.c
src-y		+= arch/xmega/twi.c
src-y		+= arch/xmega/usart.c

MACH		:= $(subst $(quote),,$(CONFIG_PLATFORM_NAME))

src-dirs	+= arch/xmega/mach-$(MACH)
-include arch/xmega/mach-$(MACH)/Makefile
