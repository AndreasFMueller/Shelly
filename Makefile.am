#
# Makefile for the Shelly project
#
# (c) 2025 Prof Dr Andreas Müller
#
EXTRA_DIST =

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

pkgdata_DATA = shellyd.config

test:	shellyd shellyd.config
	./shellyd --foreground --debug \
		--config=shellyd.config 

