AC_INIT(helloworld, 0.1, your@email.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/])
AM_CONFIG_HEADER([src/config.h])

AC_DEFINE([PROGNAME],["helloworld"],["Simple helloworld program using libpackedobjects"])

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

# Check for libpackedobjects library
PKG_CHECK_MODULES([LIBPACKEDOBJECTS], [libpackedobjects >= 0.0.6])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
