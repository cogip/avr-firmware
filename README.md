# Cogip AVR firmwares

## Build status
[![Build Status](https://travis-ci.org/cogip/avr-firmware.svg?branch=master)](https://travis-ci.org/cogip/avr-firmware)

## Dependencies

To install AVR toolchain on debian/ubuntu:

```bash
$ sudo apt-get install avrdude gcc-avr binutils-avr avr-libc
```

To install AVR toolchain on Arch Linux:

```bash
$ pacman -Sy avr-binutils avr-gcc avr-libc avrdude
```

## Build

```bash
$ make defconfig
$ make
```

## Alternative targets

Other default configuration are archived, users can configure or select a pre-defined firmware flavor:

```bash
$ make menuconfig # to launch ncurses interface and select platform & build configuration
$ make geonobot_defconfig # to set configuration for Geonobot (2015) firwmare generation
$ make cortex_defconfig # to set configuration for Cortex (2017) firmware generation
```

Then, simply run `make` to build the firmware.

## Install firmware in Flash memory

Assuming you dispose of an AVR Isp mkII programmer, you can use following to program the flash memory:

```bash
$ sudo avrdude -v -v -p atxmega128a1 -c avrispmkII -U flash:w:geonobot.hex:i
```

# External references

- [Atmel official AVR Toolchain web page](http://www.atmel.com/tools/atmelavrtoolchainforlinux.aspx)
- [A paper on recursive Makefile issue](http://aegis.sourceforge.net/auug97.pdf)
- [KConfig-frontend download page](http://ymorin.is-a-geek.org/download/kconfig-frontends/)

Kconfig usage examples:

- [Utiliser Kconfig dans ses projets](http://www.linuxembedded.fr/2013/01/utiliser-kconfig-dans-ses-projets/)
- [BitThunder OS based on Kconfig-frontend for setup](http://bitthunder.org/docs/quick-start/)
- [A kbuild skeleton we looked at initially](https://github.com/masahir0y/kbuild_skeleton)

