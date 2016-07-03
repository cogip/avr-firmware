abs-path	:= arch/xmega

src-y		+= $(abs-path)/adc.c
src-y		+= $(abs-path)/clksys.c
src-y		+= $(abs-path)/gpio.c
src-y		+= $(abs-path)/qdec.c
src-y		+= $(abs-path)/timer.c
src-y		+= $(abs-path)/twi.c
src-y		+= $(abs-path)/usart.c

MACH		:= $(subst $(quote),,$(CONFIG_PLATFORM_NAME))

src-dirs	+= $(abs-path)/mach-$(MACH)
-include $(abs-path)/mach-$(MACH)/Makefile
