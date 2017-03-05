scripts-dir	:= scripts

# -----------------------------------------------------------------------------
# Pretty display of command display or verbose: make V=1
#  this part come from Linux kernel main Makefile
ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# printing commands (catch from Kbuild.include)
dot	:= .
quote   := "
squote  := '
escsq = $(subst $(squote),'\$(squote)',$1)
echo-cmd = $(if $($(quiet)cmd_$(1)),\
	echo '  $(call escsq,$($(quiet)cmd_$(1)))$(echo-why)';)
cmd = @$(echo-cmd) $(cmd_$(1))

# -----------------------------------------------------------------------------
# Main targets

# targets are split in sets: build (default), config or clean.
# following sections are thus exclusives regarding command line goals.
build-targets		:= 1
config-targets		:= 0
clean-targets		:= 0
ifneq ($(filter config %config,$(MAKECMDGOALS)),)
        build-targets	:= 0
        config-targets	:= 1
endif
ifneq ($(filter clean mrproper,$(MAKECMDGOALS)),)
        build-targets	:= 0
        clean-targets	:= 1
endif

# -----------------------------------------------------------------------------
# Config targets
ifeq ($(config-targets),1)

LD_LIBRARY_PATH		:= $(scripts-dir)/bin-$(shell uname -m)
export LD_LIBRARY_PATH

define silentoldconfig
        mkdir -p include/config include/generated; \
        $(scripts-dir)/bin-$(shell uname -m)/kconfig-conf --silentoldconfig Kconfig
endef

.PHONY: defconfig
defconfig:
	$(Q)$(scripts-dir)/bin-$(shell uname -m)/kconfig-conf \
		--alldefconfig Kconfig
	$(Q)$(silentoldconfig)

%_defconfig:
	$(Q)if [ ! -f configs/$@ ]; then \
		echo "configs/$@ not founded!"; \
		exit 1; \
	fi
	$(Q)$(scripts-dir)/bin-$(shell uname -m)/kconfig-conf \
		--defconfig=configs/$@ Kconfig
	$(Q)$(silentoldconfig)

.PHONY: menuconfig
menuconfig:
	$(Q)$(scripts-dir)/bin-$(shell uname -m)/kconfig-mconf Kconfig
	$(Q)$(silentoldconfig)
endif

# -----------------------------------------------------------------------------
# Clean targets
ifeq ($(clean-targets),1)

.PHONY: clean
clean:
	$(Q)find . -name *.d -exec rm '{}' \;
	$(Q)find . -name *.o -exec rm '{}' \;
	$(Q)rm -f *.eep *.elf *.hex *.elf.map

.PHONY: mrproper
mrproper: clean
	$(Q)rm -Rf include/config include/generated
	$(Q)rm -f .config
endif

# -----------------------------------------------------------------------------
# Build targets
ifeq ($(build-targets),1)

-include include/config/auto.conf

ARCH			:= $(subst $(quote),,$(CONFIG_ARCH))
# list of sources dirs which contains a Makefile included from the current one:
src-dirs 		:= arch/$(ARCH) core drivers machines

MACH			:= $(subst $(quote),,$(CONFIG_PLATFORM_NAME))
binname			:= $(MACH)

.PHONY: all
all: $(binname).hex $(binname).eep

include/config/auto.conf:
	@echo "Configuration file not found."
	@echo "You need to configure before building. Available targets:"
	@echo
	@echo "    make defconfig"
	@echo "    make menuconfig"
	@echo
	@false

# aliases for CONFIG_ variables
CROSS_COMPILE	:= $(subst $(quote),,$(CONFIG_CROSS_COMPILE))
CC		:= $(CROSS_COMPILE)gcc
OBJCOPY		:= $(CROSS_COMPILE)objcopy
SIZE		:= $(CROSS_COMPILE)size
export CC

MCU		:= $(subst $(quote),,$(CONFIG_MCU))
F_CPU		:= $(CONFIG_F_CPU)

# look for include files in each of the modules
CFLAGS 		+= $(patsubst %,-I%,$(src-dirs))

CFLAGS		+= -Iarch/include
ifneq ($(MCU),)
MCUCCFLAGS	:= -mmcu=$(MCU)
else
MCUCCFLAGS	:=
endif
CFLAGS		+= $(MCUCCFLAGS) -DF_CPU=$(CONFIG_F_CPU) -std=c99
CFLAGS		+= -Wall -fpack-struct -fshort-enums -ffunction-sections \
			-fdata-sections -funsigned-char -funsigned-bitfields \
			-include include/generated/autoconf.h

ifeq ($(ARCH),unix)
CFLAGS		+= -O0 -g -D_BSD_SOURCE
LDFLAGS		:= -lrt -lm
else
CFLAGS		+= -Os
LDFLAGS		:= -lm
ifneq ($(CONFIG_ENABLE_LOGGING),)
LDFLAGS		+= -Wl,-u,vfprintf -lprintf_flt
LDFLAGS		+= -Wl,-u,vfscanf -lscanf_flt
endif
endif

# each module will add to this
src-y :=

# include the description for each module
include $(patsubst %,%/Makefile,$(src-dirs))

# determine the object files
objs := \
	$(patsubst %.c,%.o, \
	$(filter %.c,$(src-y)))

# from 'file.c' files in $(src-y), to '.file.d' in deps-y list
deps-y		:= $(foreach dep, $(src-y), \
			$(join $(dir $(dep)),$(dot)$(notdir $(dep:.c=.d))) )

# hex file generation
quiet_cmd_objcpy_hex_elf = HEX     $@
      cmd_objcpy_hex_elf = $(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $< $@

quiet_cmd_objcpy_eep_elf = EEP     $@
      cmd_objcpy_eep_elf = $(OBJCOPY) -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex $< $@

$(binname).hex: $(binname).elf
	$(call cmd,objcpy_hex_elf)

$(binname).eep: $(binname).elf
	$(call cmd,objcpy_eep_elf)

# link the program
quiet_cmd_linker = LD      $@
      cmd_linker = $(CC) -Wl,-Map,$@.map $(MCUCCFLAGS) -o $@ $(objs) $(LDFLAGS)

# statistics
quiet_cmd_size_elf = SIZE    $@
      cmd_size_elf = $(SIZE) --format=avr --mcu=$(MCU) $@

$(binname).elf: $(deps-y) $(objs)
	$(call cmd,linker)
ifeq ($(ARCH),xmega)
	$(call cmd,size_elf)
endif

# include the C include dependencies
-include $(deps-y)

# calculate C include dependencies
$(deps-y): include/config/auto.conf
$(dot)%.d: %.c
	$(Q)$(scripts-dir)/depends.sh `dirname $*.c` $(CFLAGS) $*.c > $@

# build objects
quiet_cmd_cc_o_c = CC      $@
      cmd_cc_o_c = $(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(call cmd,cc_o_c)

endif # ifeq ($(build-targets),1)
