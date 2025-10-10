#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Shelly], [0.1], [andreas.mueller@othello.ch])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
AC_CONFIG_SRCDIR([debug.h])
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_CHECK_PROGS(MARIADB_CONFIG, mariadb_config)

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([strdup strerror])

# check for curl library
CFLAGS="${CFLAGS} `curl-config --cflags`"
CXXFLAGS="${CXXFLAGS} `curl-config --cflags`"
LIBS="${LIBS} `curl-config --libs`"

# check for mysql library settings
CFLAGS="${CFLAGS} `${MARIADB_CONFIG} --cflags`"
CXXFLAGS="${CXXFLAGS} `${MARIADB_CONFIG} --cflags`"
LIBS="${LIBS} `${MARIADB_CONFIG} --libs`"

SHELLYCONFFILE=${sysconfdir}/shellyd.config
AC_SUBST(SHELLYCONFFILE)

AC_CONFIG_FILES([Makefile shellyd.8 shellyd.config.5])

AC_OUTPUT


