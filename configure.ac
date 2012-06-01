AC_INIT(packedobjectsd, 0.1, team@theclashingrocks.org)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/])
AM_CONFIG_HEADER([src/config.h])

AC_DEFINE([PROGNAME],["packedobjectsd"],["light-weight secure messaging framework"])

AC_DEFINE([DEFAULT_HOST],["127.0.0.1"],["the default zeromq server"])
AC_DEFINE([DEFAULT_PORT],[8001],["the default zeromq broker port"])
AC_DEFINE([DEFAULT_GROUP],["world"],["the group everyone can join"])
AC_DEFINE([DEFAULT_TYPE],["both"],["the default client type to run"])

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

# check for gthread
PKG_CHECK_MODULES(GTHREAD, gthread-2.0)

# check for glib
PKG_CHECK_MODULES(GLIB, [glib-2.0])

# check for zeromq
PKG_CHECK_MODULES(LIBZMQ, [libzmq])


AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
