#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([checkit], [0.3], [dennisk@netspace.net.au])
AM_INIT_AUTOMAKE()
AC_OUTPUT(Makefile src/Makefile doc/Makefile man/Makefile)
AC_CONFIG_SRCDIR([src/checkit.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_OUTPUT
