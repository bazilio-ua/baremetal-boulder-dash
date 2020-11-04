# Makefile

CROSS_COMPILE := /opt/gcc-arm-none-eabi-6-2017-q1-update/bin/arm-none-eabi-

COMPILER := gcc
SHORT_ENUMS := n

#CPU := rpi1
CPU := host
BLD_TARGET := bolder-dash
BLD_TYPE := debug

CSRC += boulder_dash.c

INC += .

ifeq ($(CPU),rpi1)
PROJ_DIRS := rpi1
endif
ifeq ($(CPU),host)
PROJ_DIRS := host
EXTRA_LIBS := -lSDL2
endif

include makefiles/main.mk

distclean:
	rm -rf build
