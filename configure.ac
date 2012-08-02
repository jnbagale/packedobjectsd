AC_INIT(packedobjectsd, 0.1, jnbagale@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/])
AM_CONFIG_HEADER([src/config.h])

AC_DEFINE([PROGNAME],["packedobjectsd"],["light-weight secure messaging framework"])
AC_DEFINE([DEFAULT_SERVER_ADDRESS],["127.0.0.1"],["the default lookup server address"])
AC_DEFINE([DEFAULT_SERVER_PORT],[5555],["the default lookup server port"])
AC_DEFINE([DEFAULT_PORT_OUT],[8100],["the default zeromq broker outbound port: for subscribers"])
AC_DEFINE([DEFAULT_PORT_IN],[5556],["the default zeromq broker inbound port: for publishers"])
AC_DEFINE([DEFAULT_TYPE],["both"],["the default node type: pub, sub or both"])
AC_DEFINE([DEFAULT_RECV_FREQ],[1000],["the default receiving frequency for subscribers in milliseconds"])
AC_DEFINE([DEFAULT_SEND_FREQ],[1000],["the default sending frequency for publishers in milliseconds"])
AC_DEFINE([DATABASE],["packedobjectsd.db"],["the default database for lookup server"])
AC_DEFINE([MAX_BUFFER],[99],["the maximum size for address buffer"])
AC_DEFINE([MAX_ADDRESS],[20],["the maximum size for network address"])

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

# Check for berkeley db library and header
#AC_SEARCH_LIBS([db_create], [db], [have_libdb=yes])
AC_CHECK_LIB(db, db_create, , AC_MSG_ERROR(*** berkeley db library not found))
AC_CHECK_HEADERS(db.h)


# required libraries pthreads
AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(*** pthread library not found))
AC_CHECK_HEADERS(pthread.h,, [AC_MSG_ERROR([pthread.h required])])

# check for glib
PKG_CHECK_MODULES(GLIB, [glib-2.0])

# check for zeromq
PKG_CHECK_MODULES(LIBZMQ, [libzmq])

# check for libxml2
PKG_CHECK_MODULES(LIBXML2, [libxml-2.0])



AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
