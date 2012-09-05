
INCLUDES = -Wall -g $(LIBZMQ_CFLAGS) $(LIBXML2_CFLAGS) $(LIBPACKEDOBJECTS_CFLAGS)

lib_LTLIBRARIES = libpackedobjectsd.la

libpackedobjectsd_la_LIBADD = $(LIBZMQ_LIBS) $(LIBXML2_LIBS) $(LIBPACKEDOBJECTS_LIBS)

libpackedobjectsd_la_SOURCES = 	packedobjectsd.c packedobjectsd.h message.c message.h \
				address.c address.h broker.c broker.h xmlutils.c xmlutils.h	

library_includedir = $(includedir)/packedobjectsd

library_include_HEADERS = packedobjectsd.h message.h broker.h xmlutils.h

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = $(top_builddir)/pkgconfig/packedobjectsd.pc

check_PROGRAMS = packedobjectsdtest

packedobjectsdtest_SOURCES = main.c

packedobjectsdtest_LDADD = $(lib_LTLIBRARIES)

