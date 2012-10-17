AC_INIT(libpackedobjectsd, 0.1, jnbagale@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/])
AM_CONFIG_HEADER([src/config.h])

AC_DEFINE([PROGNAME],["libpackedobjectsd"],["lightweight XML messaging library"])

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


# Check for pthreads library and header
AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(*** pthread library not found))
AC_CHECK_HEADERS(pthread.h,, [AC_MSG_ERROR([pthread.h required])])

# Check for cryptography library and header
AC_CHECK_LIB([crypt], [crypt], [LIBS="$LIBS -lcrypt"])
AC_CHECK_HEADERS(crypt.h,, [AC_MSG_ERROR([crypt.h required])])

# check for zeromq
PKG_CHECK_MODULES(LIBZMQ, [libzmq])

# check for libxml2
PKG_CHECK_MODULES(LIBXML2, [libxml-2.0])

# check for libpackedobjects
PKG_CHECK_MODULES(LIBPACKEDOBJECTS, [libpackedobjects])

# just create shared library
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
