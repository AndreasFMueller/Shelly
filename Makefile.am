#
# Makefile for the Shelly project
#
# (c) 2025 Prof Dr Andreas Müller
#
EXTRA_DIST =

noinst_LTLIBRARIES = libshelly.la

libshelly_la_SOURCES = 							\
	debug.cpp							\
	format.cpp

noinst_HEADERS =							\
	debug.h								\
	format.h

bin_PROGRAMS = shellyd

shellyd_SOURCES = shellyd.cpp
shellyd_DEPENDENCIES = libshelly.la
shellyd_LDFLAGS = -L. -lshelly

test:	shellyd shellyd.config
	./shellyd --foreground --debug \
		--config=shelly.config 

