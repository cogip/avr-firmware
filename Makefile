ARCH		:= xmega
MCU		:= atxmega128a1
CROSS_COMPILE	:= avr-
CC		:= $(CROSS_COMPILE)gcc
OBJCOPY		:= $(CROSS_COMPILE)objcopy
SIZE		:= $(CROSS_COMPILE)size

# list of sources dirs which contains a Makefile included from the current one:
src-dirs 	:= arch/$(ARCH) core
scripts-dir	:= scripts

# target file basename
binname		:= progname

export CC

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

# printing commands (from Kbuild.include)
squote  := '
escsq = $(subst $(squote),'\$(squote)',$1)
echo-cmd = $(if $($(quiet)cmd_$(1)),\
	echo '  $(call escsq,$($(quiet)cmd_$(1)))$(echo-why)';)
cmd = @$(echo-cmd) $(cmd_$(1))
# -----------------------------------------------------------------------------

# look for include files in each of the modules
CFLAGS 		+= $(patsubst %,-I%,$(src-dirs))

CFLAGS		+= -Iinclude/ -Iarch/
CFLAGS		+= -mmcu=$(MCU) -DF_CPU=32000000
CFLAGS		+= -Wall -Os -fpack-struct -fshort-enums -ffunction-sections \
			-fdata-sections -funsigned-char -funsigned-bitfields

# extra libraries if required
LIBS :=

# each module will add to this
src-y :=

# include the description for each module
include $(patsubst %,%/Makefile,$(src-dirs))

# determine the object files
objs := \
	$(patsubst %.c,%.o, \
	$(filter %.c,$(src-y)))

deps-y		:= $(patsubst %.c,%.d,$(src-y))

# -----------------------------------------------------------------------------
# Main target
#

.PHONY: all
all: $(binname).hex

# hex file generation
quiet_cmd_objcpy_hex_elf = HEX     $@
      cmd_objcpy_hex_elf = $(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $< $@

$(binname).hex: $(binname).elf
	$(call cmd,objcpy_hex_elf)

# link the program
quiet_cmd_linker = LD      $@
      cmd_linker = $(CC) -Wl,-Map,$@.map -mmcu=$(MCU) -o $@ $(objs) $(LIBS)

# statistics
quiet_cmd_size_elf = SIZE    $@
      cmd_size_elf = $(SIZE) --format=avr --mcu=$(MCU) $@

$(binname).elf: $(deps-y) $(objs)
	$(call cmd,linker)
	$(call cmd,size_elf)

# include the C include dependencies
#-include $(patsubst %,.%, $(objs:.o=.d))
-include $(objs:.o=.d)

# calculate C include dependencies
%.d: %.c
	@$(scripts-dir)/depends.sh `dirname $*.c` $(CFLAGS) $*.c > $@

quiet_cmd_cc_o_c = CC      $@
      cmd_cc_o_c = $(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(call cmd,cc_o_c)

.PHONY: clean
clean:
	@find . -name *.d -exec rm '{}' \;
	@find . -name *.o -exec rm '{}' \;
	@rm -f $(binname).elf $(binname).hex $(binname).elf.map

