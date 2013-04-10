AC_INIT(libpackedobjectsd, 0.0.3, jnbagale@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/])
AM_CONFIG_HEADER([src/config.h])

AC_DEFINE([PROGNAME],["libpackedobjectsd"],["lightweight XML messaging library"])
AC_DEFINE([POD_SCHEMA],["/usr/local/share/libpackedobjectsd/packedobjectsd.xsd"],["schema used for initialisation to server"])

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

# Check for pthreads library and header
AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(*** pthread library not found))
AC_CHECK_HEADERS(pthread.h,, [AC_MSG_ERROR([pthread.h required])])

# Check for cryptography library and header
AC_CHECK_LIB([crypt], [crypt], [LIBS="$LIBS -lcrypt"])
AC_CHECK_HEADERS(crypt.h,, [AC_MSG_ERROR([crypt.h required])])

# check for uuid
PKG_CHECK_MODULES(UUID, [uuid])

# check for zeromq
PKG_CHECK_MODULES(LIBZMQ, [libzmq >= 3.2])

# check for libxml2
PKG_CHECK_MODULES(LIBXML2, [libxml-2.0])

# check for libpackedobjects
PKG_CHECK_MODULES(LIBPACKEDOBJECTS, [libpackedobjects >= 0.0.6])

# just create shared library
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
