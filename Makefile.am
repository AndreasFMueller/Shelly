#
# Makefile for the Shelly project
#
# (c) 2025 Prof Dr Andreas Müller
#
EXTRA_DIST = shellyd.config shelly.service shelly.xml shellyd.8.in \
	shellyd.config.5.in

noinst_DATA = shelly.service

SHELLYCONFFILE = \"@SHELLYCONFFILE@\"
AM_CPPFLAGS = -DSHELLYCONFFILE=$(SHELLYCONFFILE)

noinst_LTLIBRARIES = libshelly.la

libshelly_la_SOURCES = 							\
	configuration.cpp						\
	loop.cpp							\
	database.cpp							\
	debug.cpp							\
	format.cpp

noinst_HEADERS =							\
	json.hpp							\
	common.h							\
	configuration.h							\
	loop.h								\
	database.h							\
	debug.h								\
	format.h

bin_PROGRAMS = shellyd

shellyd_SOURCES = shellyd.cpp
shellyd_DEPENDENCIES = libshelly.la
shellyd_LDFLAGS = -L. -lshelly

man_MANS = shellyd.8 shellyd.config.5

pkgdata_DATA = shellyd.config shelly.xml

test:	shellyd shellyd.config
	./shellyd --foreground --debug \
		--config=shellyd.config 

