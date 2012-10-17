
INCLUDES = -Wall -g $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS) $(LIBPACKEDOBJECTS_CFLAGS)

lib_LTLIBRARIES = libpackedobjectsd.la

libpackedobjectsd_la_LIBADD = $(LIBZMQ_LIBS) $(LIBXML2_LIBS) $(LIBPACKEDOBJECTS_LIBS)

libpackedobjectsd_la_SOURCES = 	packedobjectsd.c packedobjectsd.h message.c message.h \
				address.c address.h request.c request.h xmlutils.c xmlutils.h \
				$(top_builddir)/pkgconfig/packedobjectsd.pc

library_includedir = $(includedir)/packedobjectsd

library_include_HEADERS = packedobjectsd.h message.h request.h xmlutils.h config.h

check_PROGRAMS = packedobjectsdtest

packedobjectsdtest_SOURCES = main.c

packedobjectsdtest_LDADD = $(lib_LTLIBRARIES)

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = $(top_builddir)/pkgconfig/packedobjectsd.pc
