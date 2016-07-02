# Geonobot AVR firmware repackaged

## Build status
[![Build Status](https://travis-ci.org/cogip/avr-firmware.svg?branch=master)](https://travis-ci.org/cogip/avr-firmware)

## Dependencies

To install AVR toolchain on debian/ubuntu:

```bash
sudo apt-get install avrdude gcc-avr binutils-avr avr-libc
```

## Build

Thats obvious.

# External references

- [Atmel official AVR Toolchain web page](http://www.atmel.com/tools/atmelavrtoolchainforlinux.aspx)
- [A paper on recursive Makefile issue](http://aegis.sourceforge.net/auug97.pdf)
- [KConfig-frontend download page](http://ymorin.is-a-geek.org/download/kconfig-frontends/)

Kconfig usage examples:

- [Utiliser Kconfig dans ses projets](http://www.linuxembedded.fr/2013/01/utiliser-kconfig-dans-ses-projets/)
- [BitThunder OS based on Kconfig-frontend for setup](http://bitthunder.org/docs/quick-start/)
- [A kbuild skeleton we looked at initially](https://github.com/masahir0y/kbuild_skeleton)

