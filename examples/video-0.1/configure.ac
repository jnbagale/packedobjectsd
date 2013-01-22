AC_INIT(video, 0.1, jnbagale@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/])
AM_CONFIG_HEADER([src/config.h])

AC_DEFINE([PROGNAME],["video"],["simple video program using packedobjectsd library"])

# --enable-debug flag
AC_ARG_ENABLE(debug,
    [  --enable-debug     Turn on debugging [[default=no]]],
    enable_debug="$enableval",
    enable_debug=no)

if test x$enable_debug = xyes ; then
    AC_DEFINE([DEBUG_MODE], [], [true])
fi


# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

# check for libxml2
#PKG_CHECK_MODULES(LIBXML2, [libxml-2.0])

# check for libpackedobjects
PKG_CHECK_MODULES(LIBPACKEDOBJECTSD, [libpackedobjectsd])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
